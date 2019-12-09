/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

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

#include "genericvdvextractor_p.h"

#include <KItinerary/JsonLdDocument>
#include <KItinerary/VdvTicket>
#include <KItinerary/VdvTicketParser>

#include <KLocalizedString>

#include <QByteArray>
#include <QJsonArray>
#include <QJsonObject>

using namespace KItinerary;

QJsonArray GenericVdvExtractor::extract(const QByteArray &data)
{
    VdvTicketParser p;
    if (!p.parse(data)) {
        return {};
    }

    const auto vdv = p.ticket();

    QJsonObject org;
    org.insert(QStringLiteral("@type"), QLatin1String("Organization"));
    org.insert(QStringLiteral("identifier"), QString(QLatin1String("vdv:") + QString::number(vdv.issuerId())));
    QJsonObject trip;
    trip.insert(QStringLiteral("@type"), QLatin1String("TrainTrip"));
    trip.insert(QStringLiteral("provider"), org);
    trip.insert(QStringLiteral("departureDay"), vdv.beginDateTime().date().toString(Qt::ISODate));
    QJsonObject seat;
    seat.insert(QStringLiteral("@type"), QLatin1String("Seat"));
    switch (vdv.serviceClass()) {
        case VdvTicket::FirstClass:
        case VdvTicket::FirstClassUpgrade:
            seat.insert(QStringLiteral("seatingType"), QStringLiteral("1"));
            break;
        case VdvTicket::SecondClass:
            seat.insert(QStringLiteral("seatingType"), QStringLiteral("2"));
            break;
        default:
            break;
    }

    QJsonObject ticket;
    ticket.insert(QStringLiteral("@type"), QLatin1String("Ticket"));
    ticket.insert(QStringLiteral("ticketToken"), QString(QLatin1String("aztecbin:") + QString::fromLatin1(data.toBase64())));
    ticket.insert(QStringLiteral("ticketedSeat"), seat);
    if (vdv.serviceClass() == VdvTicket::FirstClassUpgrade) {
        ticket.insert(QStringLiteral("name"), i18n("Upgrade"));
    }

    QJsonObject res;
    res.insert(QStringLiteral("@type"), QLatin1String("TrainReservation"));
    res.insert(QStringLiteral("reservationFor"), trip);
    res.insert(QStringLiteral("reservationNumber"), vdv.ticketNumber());
    res.insert(QStringLiteral("reservedTicket"), ticket);
    res.insert(QStringLiteral("underName"), JsonLdDocument::toJson(vdv.person()));

    return {res};
}
