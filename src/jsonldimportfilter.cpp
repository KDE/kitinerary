/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "jsonldimportfilter.h"

#include <QJsonObject>

using namespace KItinerary;

static QJsonObject filterReservation(QJsonObject res)
{
    // move ticketToken to Ticket (Google vs. schema.org difference)
    const auto token = res.value(QLatin1String("ticketToken")).toString();
    if (!token.isEmpty()) {
        auto ticket = res.value(QLatin1String("reservedTicket")).toObject();
        if (ticket.isEmpty()) {
            ticket.insert(QLatin1String("@type"), QLatin1String("Ticket"));
        }
        if (!ticket.contains(QLatin1String("ticketToken"))) {
            ticket.insert(QLatin1String("ticketToken"), token);
            res.insert(QLatin1String("reservedTicket"), ticket);
        }
    }

    return res;
}

QJsonObject JsonLdImportFilter::filterObject(const QJsonObject& obj)
{
    if (obj.value(QLatin1String("@type")).toString().endsWith(QLatin1String("Reservation"))) {
        return filterReservation(obj);
    }
    return obj;
}
