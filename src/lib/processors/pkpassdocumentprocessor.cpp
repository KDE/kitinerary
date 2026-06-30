/*
   SPDX-FileCopyrightText: 2019-2021 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "pkpassdocumentprocessor.h"

#include <KItinerary/BusTrip>
#include <KItinerary/DocumentUtil>
#include <KItinerary/Event>
#include <KItinerary/ExtractorDocumentNodeFactory>
#include <KItinerary/ExtractorEngine>
#include <KItinerary/ExtractorFilter>
#include <KItinerary/ExtractorResult>
#include <KItinerary/Flight>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/Reservation>
#include <KItinerary/Ticket>
#include <KItinerary/TrainTrip>

#include "reservationconverter.h"
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

using namespace Qt::Literals::StringLiterals;
using namespace KItinerary;

Q_DECLARE_METATYPE(KItinerary::Internal::OwnedPtr<KPkPass::Pass>)

bool PkPassDocumentProcessor::canHandleData(const QByteArray &encodedData, QStringView fileName) const
{
    return (encodedData.startsWith("PK\x03\x04") && !QByteArrayView(encodedData).left(256).contains(".pkpass")) ||
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
    auto pass = decodedData.value<const KPkPass::Pass*>();
    if (!pass) {
        pass = decodedData.value<KPkPass::Pass*>();
    }
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
    const auto pass = node.content<const KPkPass::Pass*>();
    const auto barcodes = pass->barcodes();
    if (barcodes.empty()) {
        return;
    }

    for (const auto &barcode : barcodes) {
        // try to recover binary barcode content (which wont survive a JSON/QString roundtrip)
        const auto msg = barcode.message();
        auto data = msg.toUtf8();
        if (std::any_of(msg.begin(), msg.end(), [](QChar c) { return c.isNonCharacter() || c.isNull() || !c.isPrint(); })) {
            if (barcode.messageEncoding().compare("iso-8859-1"_L1, Qt::CaseInsensitive) == 0) {
                data = msg.toLatin1();
            }
        }
        auto child = engine->documentNodeFactory()->createNode(data);
        node.appendChild(child);
    }
}

void PkPassDocumentProcessor::destroyNode(ExtractorDocumentNode &node) const
{
    destroyIfOwned<KPkPass::Pass>(node);
}

QJSValue PkPassDocumentProcessor::contentToScriptValue(const ExtractorDocumentNode &node, QJSEngine *engine) const
{
    return engine->toScriptValue(node.content<const KPkPass::Pass*>());
}

static Airport extractSemanticTags(const KPkPass::Pass *pass, Airport airport, QLatin1StringView prefix)
{
    const auto semObj = pass->semanticTags();
    if (airport.iataCode().isEmpty()) {
        airport.setIataCode(semObj.value(prefix + "AirportCode"_L1).toString());
    }
    airport.setName(semObj.value(prefix + "AirportName"_L1).toString());
    auto addr = airport.address();
    addr.setAddressLocality(semObj.value(prefix + "CityName"_L1).toString());
    airport.setAddress(addr);
    const auto loc = semObj.value(prefix + "Location"_L1).toObject();
    airport.setGeo(GeoCoordinates{loc.value("latitude"_L1).toDouble(NAN), loc.value("longitude"_L1).toDouble(NAN)});
    return airport;
}

static FlightReservation extractSemanticTags(const KPkPass::Pass *pass, FlightReservation r)
{
    const auto semObj = pass->semanticTags();
    if (semObj.isEmpty()) {
        return r;
    }

    auto flight = r.reservationFor().value<Flight>();
    flight.setBoardingTime(QDateTime::fromString(semObj.value("originalBoardingDate"_L1).toString(), Qt::ISODate).toTimeZone(QTimeZone(semObj.value("departureLocationTimeZone"_L1).toString().toUtf8())));
    r.setBoardingGroup(semObj.value("boardingGroup"_L1).toString());

    flight.setDepartureAirport(extractSemanticTags(pass, flight.departureAirport(), "departure"_L1));
    flight.setDepartureTerminal(semObj.value("departureTerminal"_L1).toString());
    flight.setDepartureGate(semObj.value("departureGate"_L1).toString());
    flight.setDepartureTime(QDateTime::fromString(semObj.value("originalDepartureDate"_L1).toString(), Qt::ISODate).toTimeZone(QTimeZone(semObj.value("departureLocationTimeZone"_L1).toString().toUtf8())));

    flight.setArrivalAirport(extractSemanticTags(pass, flight.departureAirport(), "destination"_L1));
    flight.setArrivalTerminal(semObj.value("destinationTerminal"_L1).toString());
    flight.setDepartureTime(QDateTime::fromString(semObj.value("originalArrivalDate"_L1).toString(), Qt::ISODate).toTimeZone(QTimeZone(semObj.value("destinationLocationTimeZone"_L1).toString().toUtf8())));

    auto airline = flight.airline();
    airline.setIataCode(semObj.value("airlineCode"_L1).toString());
    flight.setAirline(airline);
    flight.setFlightNumber(semObj.value("flightNumber"_L1).toString());

    r.setReservationFor(flight);
    return r;
}

static EventReservation extractSemanticTags(const KPkPass::Pass *pass, EventReservation r)
{
    const auto semObj = pass->semanticTags();
    if (semObj.isEmpty()) {
        return r;
    }

    auto ev = r.reservationFor().value<Event>();
    if (const auto infoObj = semObj.value("eventStartDateInfo"_L1).toObject(); !infoObj.isEmpty()) {
        auto dt = QDateTime::fromString(infoObj.value("date"_L1).toString(), Qt::ISODate);
        if (const auto tz = infoObj.value("timezone"_L1).toString(); !tz.isEmpty()) {
            dt = dt.toTimeZone(QTimeZone(tz.toUtf8()));
        }
        ev.setStartDate(dt);
    } else {
        ev.setStartDate(QDateTime::fromString(semObj.value("eventStartDate"_L1).toString(), Qt::ISODate));
    }
    ev.setEndDate(QDateTime::fromString(semObj.value("eventEndDate"_L1).toString(), Qt::ISODate));

    auto loc = ev.location().value<Place>();
    loc.setName(semObj.value("venueName"_L1).toString());
    ev.setLocation(loc);

    r.setReservationFor(ev);
    return r;
}

static QList<KPkPass::Field> frontFieldsForPass(const KPkPass::Pass *pass) {
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

static Flight extractBoardingPass(const KPkPass::Pass *pass, Flight flight)
{
    // search for missing information by field key
    QString departureTerminal;
    TimeFinder timeFinder;
    const auto fields = pass->fields();
    for (const auto &field : fields) {
        // boarding time
        if (!flight.boardingTime().isValid() && (field.key().contains("boarding"_L1, Qt::CaseInsensitive) || field.label().contains("boarding"_L1, Qt::CaseInsensitive))) {
            const auto time = timeFinder.findSingularTime(field.value().toString());
            if (time.isValid()) {
                // this misses date, but the postprocessor will fill that in
                flight.setBoardingTime(QDateTime(QDate(1, 1, 1), time));
                continue;
            }
        }
        // departure gate
        if (flight.departureGate().isEmpty() && field.key().contains("gate"_L1, Qt::CaseInsensitive)) {
            const auto gateStr = field.value().toString();
            if (isPlausibleGate(gateStr)) {
                flight.setDepartureGate(gateStr);
                continue;
            }
        }
        // departure time
        if (!flight.departureTime().isValid() && field.key().contains("departure"_L1, Qt::CaseInsensitive)) {
            const auto time = timeFinder.findSingularTime(field.value().toString());
            if (time.isValid()) {
                // this misses date, but the postprocessor will fill that in
                flight.setDepartureTime(QDateTime(QDate(1, 1, 1), time));
                continue;
            }
        }

        if (field.key().contains("terminal"_L1, Qt::CaseInsensitive)) {
            if (departureTerminal.isNull()) {
                departureTerminal = field.value().toString();
            } else {
                departureTerminal = u""_s; // empty but not null, marking multiple terminal candidates
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

[[nodiscard]] static TrainReservation extractTrainTicket(const KPkPass::Pass *pass, TrainReservation res)
{
    auto trip = res.reservationFor().value<TrainTrip>();
    auto ticket = res.reservedTicket().value<Ticket>();

    TimeFinder timeFinder;
    const auto fields = pass->fields();
    for (const auto &field : fields) {
        // departure platform
        if (trip.departurePlatform().isEmpty() && field.key().contains("track"_L1, Qt::CaseInsensitive)) {
            const auto platformStr = field.value().toString();
            if (isPlausibleGate(platformStr)) {
                trip.setDeparturePlatform(platformStr);
                continue;
            }
        }
        // departure time
        if (!trip.departureTime().isValid() && field.key().contains("departure"_L1, Qt::CaseInsensitive)) {
            const auto time = timeFinder.findSingularTime(field.value().toString());
            if (time.isValid()) {
                // this misses date, but the postprocessor will fill that in
                trip.setDepartureTime(QDateTime(QDate(1, 1, 1), time));
                continue;
            }
        }
        // coach/seat
        if (ticket.ticketedSeat().seatSection().isEmpty() && field.key().contains("coach"_L1, Qt::CaseInsensitive)) {
            auto seat = ticket.ticketedSeat();
            seat.setSeatSection(field.value().toString());
            ticket.setTicketedSeat(seat);
        }
        if (ticket.ticketedSeat().seatNumber().isEmpty() && field.key().contains("seat"_L1, Qt::CaseInsensitive)) {
            auto seat = ticket.ticketedSeat();
            seat.setSeatNumber(field.value().toString());
            ticket.setTicketedSeat(seat);
        }
    }

    // "relevantDate" is the best guess for the departure time if we didn't find an explicit field for it
    if (pass->relevantDate().isValid() && !trip.departureTime().isValid()) {
        // TODO try to recover timezone?
        trip.setDepartureTime(pass->relevantDate());
    }

    // location is the best guess for the departure station geo coordinates
    auto depStation = trip.departureStation();
    auto depGeo = depStation.geo();
    if (pass->locations().size() == 1 && !depGeo.isValid()) {
        const auto loc = pass->locations().at(0);
        depGeo.setLatitude(loc.latitude());
        depGeo.setLongitude(loc.longitude());
        depStation.setGeo(depGeo);
        trip.setDepartureStation(depStation);
    }

    // organizationName is the best guess for airline name
    auto provider = trip.provider();
    if (provider.name().isEmpty()) {
        provider.setName(pass->organizationName());
        trip.setProvider(provider);
    }

    res.setReservationFor(trip);
    res.setReservedTicket(ticket);
    return res;
}

[[nodiscard]] static BusReservation extractBusTicket(const KPkPass::Pass *pass, BusReservation res)
{
    auto trip = res.reservationFor().value<BusTrip>();

    // "relevantDate" is the best guess for the departure time if we didn't find an explicit field for it
    if (pass->relevantDate().isValid() && !trip.departureTime().isValid()) {
        // TODO try to recover timezone?
        trip.setDepartureTime(pass->relevantDate());
    }

    // location is the best guess for the departure station geo coordinates
    auto depStation = trip.departureBusStop();
    auto depGeo = depStation.geo();
    if (pass->locations().size() == 1 && !depGeo.isValid()) {
        const auto loc = pass->locations().at(0);
        depGeo.setLatitude(loc.latitude());
        depGeo.setLongitude(loc.longitude());
        depStation.setGeo(depGeo);
        trip.setDepartureBusStop(depStation);
    }

    // organizationName is the best guess for the provider
    auto provider = trip.provider();
    if (provider.name().isEmpty()) {
        provider.setName(pass->organizationName());
        trip.setProvider(provider);
    }

    res.setReservationFor(trip);
    return res;
}

static void extractEventTicketPass(const KPkPass::Pass *pass, EventReservation &eventRes)
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
        priceFinder.findAll(field.valueDisplayString(), prices);
    }
    if (const auto price = priceFinder.highest(prices); price.hasResult()) {
        eventRes.setTotalPrice(price.value);
        eventRes.setPriceCurrency(price.currency);
    }

    eventRes.setReservationFor(event);
}

static Person extractPerson(const KPkPass::Pass *pass, Person person)
{
    if (const auto &nameObj = pass->semanticTags().value("passengerName"_L1).toObject(); !nameObj.isEmpty()) {
        if (const auto n = nameObj.value("familyName"_L1).toString(); !n.isEmpty()) {
            person.setFamilyName(n);
        }
        if (const auto n = nameObj.value("givenName"_L1).toString(); !n.isEmpty()) {
            person.setGivenName(n);
        }
        return person;
    }

    const auto fields = pass->fields();
    for (const auto &field : fields) {
        person = NameOptimizer::optimizeName(field.valueDisplayString(), person);
    }
    return person;
}

[[nodiscard]] static QVariant childResult(const ExtractorDocumentNode &node)
{
    if (node.childNodes().empty() || node.childNodes()[0].result().isEmpty()) {
        return {};
    }
    return node.childNodes()[0].result().result().at(0);
}

[[nodiscard]] static KPkPass::Barcode pickBestBarcode(const QList<KPkPass::Barcode> &barcodes)
{
    // prefer 2D barcodes that are more robust to scan
    const auto it = std::ranges::find_if(barcodes, [](const auto &bc) { return bc.format() == KPkPass::Barcode::Aztec || bc.format() == KPkPass::Barcode::QR; });
    return it != std::end(barcodes) ? *it : barcodes.at(0);
}

struct {
    KPkPass::Barcode::Format passBarcodeType;
    Token::TokenType tokenType;
} static constexpr const barcode_type_map[] = {
    { KPkPass::Barcode::QR, Token::QRCode },
    { KPkPass::Barcode::Aztec, Token::AztecCode },
    { KPkPass::Barcode::PDF417, Token::PDF417 },
    { KPkPass::Barcode::Code128, Token::Code128 },
    { KPkPass::Barcode::EAN13, Token::EAN13 },
    { KPkPass::Barcode::Code39, Token::Code39 },
    { KPkPass::Barcode::Codabar, Token::Codabar },
    { KPkPass::Barcode::I2of5, Token::ITF },
};

[[nodiscard]] static QString tokenFromBarcode(const KPkPass::Pass *pass)
{
    // barcode contains the ticket token
    if (const auto bcs = pass->barcodes(); !bcs.isEmpty()) {
        const auto barcode = pickBestBarcode(bcs);
        const auto it = std::ranges::find_if(barcode_type_map, [f = barcode.format()](const auto &m) { return m.passBarcodeType == f; });
        const auto type = it == std::end(barcode_type_map) ? Token::Unknown : (*it).tokenType;
        return Token::encodeTokenData(type, barcode.message());
    }
    return {};
}

template <typename T>
static void applyTicketToken(const QString &token, T &res)
{
    if (token.isEmpty()) {
        return;
    }
    auto ticket = res.reservedTicket().template value<Ticket>();
    ticket.setTicketToken(token);
    res.setReservedTicket(ticket);
}

void PkPassDocumentProcessor::preExtract(ExtractorDocumentNode &node, [[maybe_unused]] const ExtractorEngine *engine) const
{
    // barcode extraction produced more than we can handle here
    if (!node.childNodes().empty() && node.childNodes()[0].result().size() > 1) {
        return;
    }

    const auto pass = node.content<const KPkPass::Pass*>();
    QVariant res;

    switch (pass->type()) {
        case KPkPass::Pass::BoardingPass:
        {
            const auto boardingPass = qobject_cast<const KPkPass::BoardingPass*>(pass);
            if (!boardingPass) {
                return;
            }

            switch (boardingPass->transitType()) {
                case KPkPass::BoardingPass::Air: {
                    if (node.childNodes().empty() || node.childNodes()[0].result().isEmpty()) {
                        return; // without the IATA BCBP data we wont get enough details here
                    }
                    auto r = childResult(node).value<FlightReservation>();
                    r = extractSemanticTags(pass, r);
                    applyTicketToken(tokenFromBarcode(pass), r);
                    r.setReservationFor(extractBoardingPass(pass, r.reservationFor().value<Flight>()));
                    r.setUnderName(extractPerson(pass, r.underName().value<Person>()));
                    res = r;
                    break;
                }
                case KPkPass::BoardingPass::Train:
                {
                    // if this is a IATA BCBP the child node extractor will have classified this as a flight, so convert that
                    auto r = childResult(node).value<TrainReservation>();
                    if (!node.childNodes().empty() && !node.childNodes()[0].result().isEmpty() && JsonLd::isA<FlightReservation>(node.childNodes()[0].result().result().at(0))) {
                        const auto flightRes = node.childNodes()[0].result().jsonLdResult().at(0).toObject();
                        res = JsonLdDocument::apply(JsonLdDocument::fromJsonSingular(ReservationConverter::flightToTrain(flightRes)), res).value<TrainReservation>();
                    }
                    applyTicketToken(tokenFromBarcode(pass), r);
                    r = extractTrainTicket(pass, r);
                    r.setUnderName(extractPerson(pass, r.underName().value<Person>()));
                    res = r;
                    break;
                }
                case KPkPass::BoardingPass::Bus:
                {
                    auto r = childResult(node).value<BusReservation>();
                    applyTicketToken(tokenFromBarcode(pass), r);
                    r = extractBusTicket(pass, r);
                    r.setUnderName(extractPerson(pass, r.underName().value<Person>()));
                    res = r;
                    break;
                }
                case KPkPass::BoardingPass::Boat:
                {
                    auto r = childResult(node).value<BoatReservation>();
                    applyTicketToken(tokenFromBarcode(pass), r);
                    r.setUnderName(extractPerson(pass, r.underName().value<Person>()));
                    res = r;
                    break;
                }
                case KPkPass::BoardingPass::Generic:
                    return;
            }
            break;
        }
        case KPkPass::Pass::EventTicket:
        {
            auto r = childResult(node).value<EventReservation>();
            r = extractSemanticTags(pass, r);
            applyTicketToken(tokenFromBarcode(pass), r);
            extractEventTicketPass(pass, r);
            res = r;
            break;
        }
        case KPkPass::Pass::Coupon:
        case KPkPass::Pass::StoreCard:
        case KPkPass::Pass::Generic:
            return;
    }

    node.setResult(QList<QVariant>({res}));
}

void PkPassDocumentProcessor::postExtract(ExtractorDocumentNode &node, [[maybe_unused]] const ExtractorEngine *engine) const
{
    const auto pass = node.content<const KPkPass::Pass*>();
    if (pass->passTypeIdentifier().isEmpty() || pass->serialNumber().isEmpty()) {
        return;
    }

    // associate the pass with the result, so we can find the pass again for display
    auto result = node.result().result();
    for (auto &res : result) {
        DocumentUtil::addDocumentId(res, DocumentUtil::idForPkPass(pass->passTypeIdentifier(), pass->serialNumber()));
        // TODO replace this eventually with the more generic and standard compliant way above
        JsonLdDocument::writeProperty(res, "pkpassPassTypeIdentifier", pass->passTypeIdentifier());
        JsonLdDocument::writeProperty(res, "pkpassSerialNumber", pass->serialNumber());
        // pass->relevantDate() as modification time is inherently unreliable (it wont change most of the time)
        // so if we have something from an enclosing document, that's probably better
        if (node.parent().contextDateTime().isValid()) {
            JsonLdDocument::writeProperty(res, "modifiedTime", node.parent().contextDateTime().toString(Qt::ISODate));
        }
    }
    node.setResult(result);
}
