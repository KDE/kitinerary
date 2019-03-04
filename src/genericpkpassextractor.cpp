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

#include <KPkPass/Barcode>
#include <KPkPass/BoardingPass>
#include <KPkPass/Location>
#include <KPkPass/Field>

#include <QJsonObject>
#include <QTime>
#include <QVariant>

using namespace KItinerary;

static void extractBoardingPass(KPkPass::Pass *pass, QJsonObject &resFor)
{
    // "relevantDate" is the best guess for the boarding time
    if (pass->relevantDate().isValid() && !resFor.contains(QLatin1String("boardingTime"))) {
        resFor.insert(QStringLiteral("boardingTime"), pass->relevantDate().toString(Qt::ISODate));
    }
    // look for common field names containing the boarding time, if we still have no idea
    if (!resFor.contains(QLatin1String("boardingTime"))) {
        for (const auto &field : pass->fields()) {
            if (!field.key().contains(QLatin1String("boarding"), Qt::CaseInsensitive)) {
                continue;
            }
            const auto time = QTime::fromString(field.value().toString());
            if (time.isValid()) {
                // this misses date, but the postprocessor will fill that in
                resFor.insert(QStringLiteral("boardingTime"), QDateTime(QDate(1, 1, 1), time).toString(Qt::ISODate));
                break;
            }
        }
    }

    // location is the best guess for the departure airport geo coordinates
    auto depAirport = resFor.value(QLatin1String("departureAirport")).toObject();
    if (depAirport.isEmpty()) {
        depAirport.insert(QStringLiteral("@type"), QLatin1String("Airport"));
    }
    auto depGeo = depAirport.value(QLatin1String("geo")).toObject();
    if (pass->locations().size() == 1 && depGeo.isEmpty()) {
        const auto loc = pass->locations().at(0);
        depGeo.insert(QStringLiteral("@type"), QLatin1String("GeoCoordinates"));
        depGeo.insert(QStringLiteral("latitude"), loc.latitude());
        depGeo.insert(QStringLiteral("longitude"), loc.longitude());
        depAirport.insert(QStringLiteral("geo"), depGeo);
        resFor.insert(QStringLiteral("departureAirport"), depAirport);
    }

    // organizationName is the best guess for airline name
    auto airline = resFor.value(QLatin1String("airline")).toObject();
    if (airline.isEmpty()) {
        airline.insert(QStringLiteral("@type"), QLatin1String("Airline"));
    }
    if (!airline.contains(QLatin1String("name"))) {
        airline.insert(QStringLiteral("name"), pass->organizationName());
    }
    resFor.insert(QStringLiteral("airline"), airline);
}

static void extractEventTicketPass(KPkPass::Pass *pass, QJsonObject &resFor)
{
    if (!resFor.contains(QLatin1String("name"))) {
        resFor.insert(QStringLiteral("name"), pass->description());
    }

    // "relevantDate" is the best guess for the start time
    if (pass->relevantDate().isValid() && !resFor.contains(QLatin1String("startDate"))) {
        resFor.insert(QStringLiteral("startDate"), pass->relevantDate().toString(Qt::ISODate));
    }

    // location is the best guess for the venue
    auto venue = resFor.value(QLatin1String("location")).toObject();
    if (venue.isEmpty()) {
        venue.insert(QStringLiteral("@type"), QLatin1String("Place"));
    }
    auto geo = venue.value(QLatin1String("geo")).toObject();
    if (!pass->locations().isEmpty() && geo.isEmpty()) {
        const auto loc = pass->locations().at(0);
        geo.insert(QStringLiteral("@type"), QLatin1String("GeoCoordinates"));
        geo.insert(QStringLiteral("latitude"), loc.latitude());
        geo.insert(QStringLiteral("longitude"), loc.longitude());
        venue.insert(QStringLiteral("geo"), geo);
        venue.insert(QStringLiteral("name"), loc.relevantText());
        resFor.insert(QStringLiteral("location"), venue);
    }
}

void GenericPkPassExtractor::extract(KPkPass::Pass *pass, QJsonObject &result)
{
    if (result.isEmpty()) { // no previous extractor ran, so we need to create the top-level element ourselves
        QJsonObject resFor;
        if (auto boardingPass = qobject_cast<KPkPass::BoardingPass*>(pass)) {
            switch (boardingPass->transitType()) {
                case KPkPass::BoardingPass::Air:
                    result.insert(QStringLiteral("@type"), QLatin1String("FlightReservation"));
                    resFor.insert(QStringLiteral("@type"), QLatin1String("Flight"));
                    break;
                // TODO expand once we have test files for train tickets
                default:
                    return;
            }
        } else {
            switch (pass->type()) {
                case KPkPass::Pass::EventTicket:
                    result.insert(QStringLiteral("@type"), QLatin1String("EventReservation"));
                    resFor.insert(QStringLiteral("@type"), QLatin1String("Event"));
                    break;
                default:
                    return;
            }
        }
        result.insert(QStringLiteral("reservationFor"), resFor);
    }

    // extract structured data from a pkpass, if the extractor script hasn't done so already
    auto resFor = result.value(QLatin1String("reservationFor")).toObject();
    switch (pass->type()) {
        case KPkPass::Pass::BoardingPass:
            extractBoardingPass(pass, resFor);
            break;
        case KPkPass::Pass::EventTicket:
            extractEventTicketPass(pass, resFor);
            break;
        default:
            return;
    }

    // barcode contains the ticket token
    if (!pass->barcodes().isEmpty() && !result.contains(QLatin1String("reservedTicket"))) {
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
        QJsonObject ticket;
        ticket.insert(QStringLiteral("@type"), QLatin1String("Ticket"));
        ticket.insert(QStringLiteral("ticketToken"), token);
        result.insert(QStringLiteral("reservedTicket"), ticket);
    }

    result.insert(QStringLiteral("reservationFor"), resFor);

    // associate the pass with the result, so we can find the pass again for display
    if (!pass->passTypeIdentifier().isEmpty() && !pass->serialNumber().isEmpty()) {
        result.insert(QStringLiteral("pkpassPassTypeIdentifier"), pass->passTypeIdentifier());
        result.insert(QStringLiteral("pkpassSerialNumber"), pass->serialNumber());
    }
}
