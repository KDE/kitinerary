/*
   SPDX-FileCopyrightText: 2019-2021 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "pkpassdocumentprocessor.h"

#include <KItinerary/Event>
#include <KItinerary/ExtractorDocumentNodeFactory>
#include <KItinerary/ExtractorEngine>
#include <KItinerary/ExtractorFilter>
#include <KItinerary/ExtractorResult>
#include <KItinerary/Flight>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/Reservation>
#include <KItinerary/Ticket>

#include "knowledgedb/airportdb.h"
#include "text/nameoptimizer_p.h"
#include "text/pricefinder_p.h"
#include "text/timefinder_p.h"

#include <KPkPass/Barcode>
#include <KPkPass/BoardingPass>
#include <KPkPass/Location>
#include <KPkPass/Field>
#include <KPkPass/Pass>

#include <QJsonObject>
#include <QJSEngine>
#include <QTime>
#include <QTimeZone>
#include <QVariant>

using namespace KItinerary;

Q_DECLARE_METATYPE(KItinerary::Internal::OwnedPtr<KPkPass::Pass>)

bool PkPassDocumentProcessor::canHandleData(const QByteArray &encodedData, QStringView fileName) const
{
  return encodedData.startsWith("PK\x03\x04") ||
         fileName.endsWith(QLatin1StringView(".pkpass"), Qt::CaseInsensitive);
}

ExtractorDocumentNode PkPassDocumentProcessor::createNodeFromData(const QByteArray &encodedData) const
{
    auto pass = KPkPass::Pass::fromData(encodedData);
    if (!pass) {
        return {};
    }

    ExtractorDocumentNode node;
    node.setContent<Internal::OwnedPtr<KPkPass::Pass>>(pass);
    if (pass->relevantDate().isValid()) {
        node.setContextDateTime(pass->relevantDate().addDays(-1)); // go a bit back, to compensate for unknown departure timezone at this point
    }
    return node;
}

ExtractorDocumentNode PkPassDocumentProcessor::createNodeFromContent(const QVariant &decodedData) const
{
    auto pass = decodedData.value<KPkPass::Pass*>();
    if (!pass) {
        return {};
    }

    ExtractorDocumentNode node;
    node.setContent(pass);
    if (pass->relevantDate().isValid()) {
        node.setContextDateTime(pass->relevantDate().addDays(-1)); // go a bit back, to compensate for unknown departure timezone at this point
    }
    return node;
}

void PkPassDocumentProcessor::expandNode(ExtractorDocumentNode &node, const ExtractorEngine *engine) const
{
    const auto pass = node.content<KPkPass::Pass*>();
    const auto barcodes = pass->barcodes();
    if (barcodes.empty()) {
        return;
    }

    auto child = engine->documentNodeFactory()->createNode(barcodes[0].message().toUtf8());
    node.appendChild(child);
}

void PkPassDocumentProcessor::destroyNode(ExtractorDocumentNode &node) const
{
    destroyIfOwned<KPkPass::Pass>(node);
}

QJSValue PkPassDocumentProcessor::contentToScriptValue(const ExtractorDocumentNode &node, QJSEngine *engine) const
{
    return engine->toScriptValue(node.content<KPkPass::Pass*>());
}

static QList<KPkPass::Field> frontFieldsForPass(KPkPass::Pass *pass) {
    QList<KPkPass::Field> fields;
    fields += pass->headerFields();
    fields += pass->primaryFields();
    fields += pass->secondaryFields();
    fields += pass->auxiliaryFields();
    return fields;
}

static bool isAirportName(const QString &name, KnowledgeDb::IataCode iataCode)
{
    if (name.size() <= 3) {
        return false;
    }

    const auto codes = KnowledgeDb::iataCodesFromName(name);
    return std::find(codes.begin(), codes.end(), iataCode) != codes.end();
}

static bool isPlausibleGate(const QString &s)
{
    if (s.isEmpty() || s.size() > 10 || s.count(QLatin1Char('-')) > 1 || s.count(QLatin1Char(' ')) > 2) {
        return false;
    }
    for (const auto &c : s) {
        if (!c.isLetter() && !c.isDigit() && c != QLatin1Char(' ') && c != QLatin1Char(' ')) {
            return false;
        }
    }
    return true;
}

static Flight extractBoardingPass(KPkPass::Pass *pass, Flight flight)
{
    // search for missing information by field key
    QString departureTerminal;
    TimeFinder timeFinder;
    const auto fields = pass->fields();
    for (const auto &field : fields) {
        // boarding time
        if (!flight.boardingTime().isValid() &&
            field.key().contains(QLatin1StringView("boarding"),
                                 Qt::CaseInsensitive)) {
          const auto time =
              timeFinder.findSingularTime(field.value().toString());
          if (time.isValid()) {
            // this misses date, but the postprocessor will fill that in
            flight.setBoardingTime(QDateTime(QDate(1, 1, 1), time));
            continue;
          }
        }
        // departure gate
        if (flight.departureGate().isEmpty() &&
            field.key().contains(QLatin1StringView("gate"),
                                 Qt::CaseInsensitive)) {
          const auto gateStr = field.value().toString();
          if (isPlausibleGate(gateStr)) {
            flight.setDepartureGate(gateStr);
            continue;
          }
        }
        // departure time
        if (!flight.departureTime().isValid() &&
            field.key().contains(QLatin1StringView("departure"),
                                 Qt::CaseInsensitive)) {
          const auto time =
              timeFinder.findSingularTime(field.value().toString());
          if (time.isValid()) {
            // this misses date, but the postprocessor will fill that in
            flight.setDepartureTime(QDateTime(QDate(1, 1, 1), time));
            continue;
          }
        }

        if (field.key().contains(QLatin1StringView("terminal"),
                                 Qt::CaseInsensitive)) {
          if (departureTerminal.isNull()) {
            departureTerminal = field.value().toString();
          } else {
            departureTerminal = QStringLiteral(
                ""); // empty but not null, marking multiple terminal candidates
          }
        }
    }

    if (flight.departureTerminal().isEmpty() && !departureTerminal.isEmpty()) {
        flight.setDepartureTerminal(departureTerminal);
    }

    // "relevantDate" is the best guess for the boarding time if we didn't find an explicit field for it
    if (pass->relevantDate().isValid() && !flight.boardingTime().isValid()) {
        const auto tz = KnowledgeDb::timezoneForAirport(KnowledgeDb::IataCode{flight.departureAirport().iataCode()});
        if (tz.isValid()) {
            flight.setBoardingTime(pass->relevantDate().toTimeZone(tz));
        } else {
            flight.setBoardingTime(pass->relevantDate());
        }
    }

   // search for missing information in field content
    const auto depIata = KnowledgeDb::IataCode(flight.departureAirport().iataCode());
    const auto arrIata = KnowledgeDb::IataCode(flight.arrivalAirport().iataCode());
    const auto frontFields = frontFieldsForPass(pass);
    for (const auto &field : frontFields) {
        // full airport names
        if (flight.departureAirport().name().isEmpty()) {
            if (isAirportName(field.value().toString(), depIata)) {
                auto airport = flight.departureAirport();
                airport.setName(field.value().toString());
                flight.setDepartureAirport(airport);
            } else if (isAirportName(field.label(), depIata)) {
                auto airport = flight.departureAirport();
                airport.setName(field.label());
                flight.setDepartureAirport(airport);
            }
        }
        if (flight.arrivalAirport().name().isEmpty()) {
            if (isAirportName(field.value().toString(), arrIata)) {
                auto airport = flight.arrivalAirport();
                airport.setName(field.value().toString());
                flight.setArrivalAirport(airport);
            } else if (isAirportName(field.label(), arrIata)) {
                auto airport = flight.arrivalAirport();
                airport.setName(field.label());
                flight.setArrivalAirport(airport);
            }
        }
    }

    // location is the best guess for the departure airport geo coordinates
    auto depAirport = flight.departureAirport();
    auto depGeo = depAirport.geo();
    if (pass->locations().size() == 1 && !depGeo.isValid()) {
        const auto loc = pass->locations().at(0);
        depGeo.setLatitude(loc.latitude());
        depGeo.setLongitude(loc.longitude());
        depAirport.setGeo(depGeo);
        flight.setDepartureAirport(depAirport);
    }

    // organizationName is the best guess for airline name
    auto airline = flight.airline();
    if (airline.name().isEmpty()) {
        airline.setName(pass->organizationName());
        flight.setAirline(airline);
    }

    return flight;
}

static void extractEventTicketPass(KPkPass::Pass *pass, EventReservation &eventRes)
{
    auto event = eventRes.reservationFor().value<Event>();

    if (event.name().isEmpty()) {
        event.setName(pass->description());
    }

    // "relevantDate" is the best guess for the start time
    if (pass->relevantDate().isValid() && !event.startDate().isValid()) {
        event.setStartDate(pass->relevantDate());

        // "expirationDate" is the best guess for the end time
        if (pass->expirationDate().isValid() && pass->relevantDate().date() == pass->expirationDate().date() &&
            pass->expirationDate() > pass->relevantDate() && !event.endDate().isValid()) {
            event.setEndDate(pass->expirationDate());
        }
    }

    // location is the best guess for the venue
    auto venue = event.location().value<Place>();
    auto geo = venue.geo();
    if (!pass->locations().isEmpty() && !geo.isValid()) {
        const auto loc = pass->locations().at(0);
        geo.setLatitude(loc.latitude());
        geo.setLongitude(loc.longitude());
        venue.setGeo(geo);
        if (venue.name().isEmpty()) {
            venue.setName(loc.relevantText());
        }
        event.setLocation(venue);
    }

    // search for prices
    PriceFinder priceFinder;
    std::vector<PriceFinder::Result> prices;
    const auto fields = pass->fields();
    for (const auto &field : fields) {
        priceFinder.findAll(field.value().toString(), prices);
    }
    if (const auto price = priceFinder.highest(prices); price.hasResult()) {
        eventRes.setTotalPrice(price.value);
        eventRes.setPriceCurrency(price.currency);
    }

    eventRes.setReservationFor(event);
}

static Person extractPerson(const KPkPass::Pass *pass, Person person)
{
    const auto fields = pass->fields();
    for (const auto &field : fields) {
        person = NameOptimizer::optimizeName(field.valueDisplayString(), person);
    }
    return person;
}

void PkPassDocumentProcessor::preExtract(ExtractorDocumentNode &node, [[maybe_unused]] const ExtractorEngine *engine) const
{
    const auto pass = node.content<KPkPass::Pass*>();
    QJsonObject result;
    if (auto boardingPass = qobject_cast<KPkPass::BoardingPass*>(pass)) {
        switch (boardingPass->transitType()) {
            case KPkPass::BoardingPass::Air:
              result.insert(QStringLiteral("@type"),
                            QLatin1StringView("FlightReservation"));
              break;
            case KPkPass::BoardingPass::Train:
              result.insert(QStringLiteral("@type"),
                            QLatin1StringView("TrainReservation"));
              break;
            case KPkPass::BoardingPass::Bus:
              result.insert(QStringLiteral("@type"),
                            QLatin1StringView("BusReservation"));
              break;
            // TODO expand once we have test files for other types
            default:
                break;
        }
    } else {
        switch (pass->type()) {
            case KPkPass::Pass::EventTicket:
              result.insert(QStringLiteral("@type"),
                            QLatin1StringView("EventReservation"));
              break;
            default:
                return;
        }
    }

    // barcode contains the ticket token
    if (!pass->barcodes().isEmpty()) {
        const auto barcode = pass->barcodes().at(0);
        QString token;
        switch (barcode.format()) {
            case KPkPass::Barcode::QR:
              token += QLatin1StringView("qrCode:");
              break;
            case KPkPass::Barcode::Aztec:
              token += QLatin1StringView("aztecCode:");
              break;
            default:
                break;
        }
        token += barcode.message();
        QJsonObject ticket =
            result.value(QLatin1StringView("reservedTicket")).toObject();
        ticket.insert(QStringLiteral("@type"), QLatin1StringView("Ticket"));
        ticket.insert(QStringLiteral("ticketToken"), token);
        result.insert(QStringLiteral("reservedTicket"), ticket);
    }

    // explicitly merge with the decoded barcode data, as this would other wise not match
    auto res = JsonLdDocument::fromJsonSingular(result);
    if (JsonLd::isA<FlightReservation>(res)) {
        // if this doesn't contain a single IATA BCBP we wont be able to get sufficient information out of this
        if (node.childNodes().size() != 1 || node.childNodes()[0].result().size() != 1) {
            return;
        }
        res = JsonLdDocument::apply(node.childNodes()[0].result().result().at(0), res).value<FlightReservation>();
    }

    // extract structured data from a pkpass, if the extractor script hasn't done so already
    switch (pass->type()) {
        case KPkPass::Pass::BoardingPass:
        {
            if (auto boardingPass = qobject_cast<KPkPass::BoardingPass*>(pass)) {
                switch (boardingPass->transitType()) {
                    case KPkPass::BoardingPass::Air:
                    {
                        auto flightRes = res.value<FlightReservation>();
                        flightRes.setReservationFor(extractBoardingPass(pass, flightRes.reservationFor().value<Flight>()));
                        flightRes.setUnderName(extractPerson(pass, flightRes.underName().value<Person>()));
                        res = flightRes;
                        break;
                    }
                    default:
                        break;
                }
            }
            break;
        }
        case KPkPass::Pass::EventTicket:
        {
            auto evRes = res.value<EventReservation>();
            extractEventTicketPass(pass, evRes);
            res = evRes;
            break;
        }
        default:
            break;
    }
    node.setResult(QList<QVariant>({res}));
}

void PkPassDocumentProcessor::postExtract(ExtractorDocumentNode &node, [[maybe_unused]] const ExtractorEngine *engine) const
{
    const auto pass = node.content<KPkPass::Pass*>();
    if (pass->passTypeIdentifier().isEmpty() || pass->serialNumber().isEmpty()) {
        return;
    }

    // associate the pass with the result, so we can find the pass again for display
    auto result = node.result().jsonLdResult();
    for (auto resV : result) {
        auto res = resV.toObject();
        res.insert(QLatin1StringView("pkpassPassTypeIdentifier"),
                   pass->passTypeIdentifier());
        res.insert(QLatin1StringView("pkpassSerialNumber"),
                   pass->serialNumber());
        // pass->relevantDate() as modification time is inherently unreliable (it wont change most of the time)
        // so if we have something from an enclosing document, that's probably better
        if (node.parent().contextDateTime().isValid()) {
          res.insert(QLatin1StringView("modifiedTime"),
                     node.parent().contextDateTime().toString(Qt::ISODate));
        }
        resV = res;
    }
    node.setResult(result);
}
