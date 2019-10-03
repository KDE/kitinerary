/*
   Copyright (c) 2019 Volker Krause <vkrause@kde.org>

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   This library is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
   License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.
*/

#include "genericpkpassextractor_p.h"

#include <KItinerary/Event>
#include <KItinerary/Flight>
#include <KItinerary/IataBcbpParser>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/Reservation>
#include <KItinerary/Ticket>

#include <knowledgedb/airportdb.h>

#include <KPkPass/Barcode>
#include <KPkPass/BoardingPass>
#include <KPkPass/Location>
#include <KPkPass/Field>

#include <QJsonObject>
#include <QTime>
#include <QTimeZone>
#include <QVariant>

using namespace KItinerary;

static Flight extractBoardingPass(KPkPass::Pass *pass, Flight flight)
{
    // "relevantDate" is the best guess for the boarding time
    if (pass->relevantDate().isValid() && !flight.boardingTime().isValid()) {
        const auto tz = KnowledgeDb::timezoneForAirport(KnowledgeDb::IataCode{flight.departureAirport().iataCode()});
        if (tz.isValid()) {
            flight.setBoardingTime(pass->relevantDate().toTimeZone(tz));
        } else {
            flight.setBoardingTime(pass->relevantDate());
        }
    }
    // look for common field names containing the boarding time, if we still have no idea
    if (!flight.boardingTime().isValid()) {
        const auto fields = pass->fields();
        for (const auto &field : fields) {
            if (!field.key().contains(QLatin1String("boarding"), Qt::CaseInsensitive)) {
                continue;
            }
            const auto time = QTime::fromString(field.value().toString());
            if (time.isValid()) {
                // this misses date, but the postprocessor will fill that in
                flight.setBoardingTime(QDateTime(QDate(1, 1, 1), time));
                break;
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

static Event extractEventTicketPass(KPkPass::Pass *pass, Event event)
{
    if (event.name().isEmpty()) {
        event.setName(pass->description());
    }

    // "relevantDate" is the best guess for the start time
    if (pass->relevantDate().isValid() && !event.startDate().isValid()) {
        event.setStartDate(pass->relevantDate());
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
    return event;
}

static QDateTime iataContextDate(KPkPass::Pass *pass, const QDateTime &context)
{
    if (!pass->relevantDate().isValid()) {
        return context;
    }
    return pass->relevantDate().addDays(-1); // go a bit back, to compensate for unknown departure timezone at this point
}

QJsonObject GenericPkPassExtractor::extract(KPkPass::Pass *pass, const QJsonObject &extracted, const QDateTime &contextDate)
{
    auto result = extracted;
    if (result.isEmpty()) { // no previous extractor ran, so we need to create the top-level element ourselves
        if (auto boardingPass = qobject_cast<KPkPass::BoardingPass*>(pass)) {
            switch (boardingPass->transitType()) {
                case KPkPass::BoardingPass::Air:
                    result.insert(QStringLiteral("@type"), QLatin1String("FlightReservation"));
                    break;
                // TODO expand once we have test files for train tickets
                default:
                    break;
            }
        } else {
            switch (pass->type()) {
                case KPkPass::Pass::EventTicket:
                    result.insert(QStringLiteral("@type"), QLatin1String("EventReservation"));
                    break;
                default:
                    return result;
            }
        }
    }

    // barcode contains the ticket token
    if (!pass->barcodes().isEmpty()) {
        const auto barcode = pass->barcodes().at(0);
        QString token;
        switch (barcode.format()) {
            case KPkPass::Barcode::QR:
                token += QLatin1String("qrCode:");
                break;
            case KPkPass::Barcode::Aztec:
                token += QLatin1String("aztecCode:");
                break;
            default:
                break;
        }
        token += barcode.message();
        QJsonObject ticket = result.value(QLatin1String("reservedTicket")).toObject();
        ticket.insert(QStringLiteral("@type"), QLatin1String("Ticket"));
        if (!ticket.contains(QLatin1String("ticketToken"))) {
            ticket.insert(QStringLiteral("ticketToken"), token);
        }
        result.insert(QStringLiteral("reservedTicket"), ticket);
    }

    // decode the barcode here already, so we have more information available for the following steps
    // also, we have additional context time information here
    auto res = JsonLdDocument::fromJson(result);
    if (JsonLd::isA<FlightReservation>(res)) {
        const auto bcbp = res.value<FlightReservation>().reservedTicket().value<Ticket>().ticketTokenData();
        const auto bcbpData = IataBcbpParser::parse(bcbp, iataContextDate(pass, contextDate).date());
        if (bcbpData.size() == 1) {
            res = JsonLdDocument::apply(bcbpData.at(0), res).value<FlightReservation>();
        }
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
            evRes.setReservationFor(extractEventTicketPass(pass, evRes.reservationFor().value<Event>()));
            res = evRes;
            break;
        }
        default:
            break;
    }

    // associate the pass with the result, so we can find the pass again for display
    result = JsonLdDocument::toJson(res);
    if (!pass->passTypeIdentifier().isEmpty() && !pass->serialNumber().isEmpty()) {
        result.insert(QStringLiteral("pkpassPassTypeIdentifier"), pass->passTypeIdentifier());
        result.insert(QStringLiteral("pkpassSerialNumber"), pass->serialNumber());
    }

    return result;
}
