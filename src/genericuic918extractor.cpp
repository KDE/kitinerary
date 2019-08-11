/*
    Copyright (C) 2018-2019 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "genericuic918extractor_p.h"

#include <KItinerary/Uic9183Parser>
#include <KItinerary/Rct2Ticket>

#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>

using namespace KItinerary;

void GenericUic918Extractor::extract(const QByteArray &data, QJsonArray &result, const QDateTime &contextDate)
{
    Uic9183Parser p;
    p.setContextDate(contextDate);
    p.parse(data);
    if (!p.isValid()) {
        return;
    }

    QJsonObject org;
    org.insert(QStringLiteral("@type"), QLatin1String("Organization"));
    org.insert(QStringLiteral("identifier"), QString(QLatin1String("uic:") + p.carrierId()));
    QJsonObject trip;
    trip.insert(QStringLiteral("@type"), QLatin1String("TrainTrip"));
    trip.insert(QStringLiteral("provider"), org);
    QJsonObject seat;
    seat.insert(QStringLiteral("@type"), QLatin1String("Seat"));

    const auto rct2 = p.rct2Ticket();
    if (rct2.isValid()) {
        switch (rct2.type()) {
            case Rct2Ticket::Reservation:
            case Rct2Ticket::TransportReservation:
            {
                trip.insert(QStringLiteral("trainNumber"), rct2.trainNumber());
                seat.insert(QStringLiteral("seatSection"), rct2.coachNumber());
                seat.insert(QStringLiteral("seatNumber"), rct2.seatNumber());
                Q_FALLTHROUGH();
            }
            case Rct2Ticket::Transport:
            {
                QJsonObject dep;
                dep.insert(QStringLiteral("@type"), QLatin1String("TrainStation"));
                dep.insert(QStringLiteral("name"), rct2.outboundDepartureStation());
                trip.insert(QStringLiteral("departureStation"), dep);

                QJsonObject arr;
                arr.insert(QStringLiteral("@type"), QLatin1String("TrainStation"));
                arr.insert(QStringLiteral("name"), rct2.outboundArrivalStation());
                trip.insert(QStringLiteral("arrivalStation"), arr);

                if (rct2.outboundDepartureTime().isValid()) {
                    trip.insert(QStringLiteral("departureDay"), rct2.outboundDepartureTime().date().toString(Qt::ISODate));
                } else {
                    trip.insert(QStringLiteral("departureDay"), rct2.firstDayOfValidity().toString(Qt::ISODate));
                }

                if (rct2.outboundDepartureTime() != rct2.outboundArrivalTime()) {
                    trip.insert(QStringLiteral("departureTime"), rct2.outboundDepartureTime().toString(Qt::ISODate));
                    trip.insert(QStringLiteral("arrivalTime"), rct2.outboundArrivalTime().toString(Qt::ISODate));
                }

                seat.insert(QStringLiteral("seatingType"), rct2.outboundClass());
                break;
            }
            default:
                break;
        }
    }

    QJsonObject ticket;
    ticket.insert(QStringLiteral("@type"), QLatin1String("Ticket"));
    ticket.insert(QStringLiteral("ticketToken"), QString(QLatin1String("aztecbin:") + QString::fromLatin1(data.toBase64())));
    ticket.insert(QStringLiteral("ticketedSeat"), seat);

    QJsonObject person;
    person.insert(QStringLiteral("@type"), QLatin1String("Person"));
    if (!rct2.passengerName().isEmpty()) {
        person.insert(QStringLiteral("name"), rct2.passengerName());
    }

    QJsonObject res;
    res.insert(QStringLiteral("@type"), QLatin1String("TrainReservation"));
    res.insert(QStringLiteral("reservationFor"), trip);
    res.insert(QStringLiteral("reservationNumber"), p.pnr());
    res.insert(QStringLiteral("reservedTicket"), ticket);
    res.insert(QStringLiteral("underName"), person);

    result.push_back(res);
}
