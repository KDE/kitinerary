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

static void filterTrainTrip(QJsonObject &trip)
{
    if (trip.value(QLatin1String("@type")).toString() != QLatin1String("TrainTrip")) {
        return;
    }

    // move TrainTrip::trainCompany to TrainTrip::provider (as defined by schema.org)
    const auto company = trip.value(QLatin1String("trainCompany")).toObject();
    if (!company.isEmpty() && !trip.contains(QLatin1String("provider"))) {
        trip.insert(QLatin1String("provider"), company);
    }
}

static void filterReservation(QJsonObject &res)
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
}

QJsonObject JsonLdImportFilter::filterObject(const QJsonObject& obj)
{
    QJsonObject res(obj);
    const auto type = obj.value(QLatin1String("@type")).toString();
    if (type.endsWith(QLatin1String("Reservation"))) {
        filterReservation(res);
    }

    if (type == QLatin1String("TrainReservation")) {
        auto train = obj.value(QLatin1String("reservationFor")).toObject();
        filterTrainTrip(train);
        if (!train.isEmpty()) {
            res.insert(QLatin1String("reservationFor"), train);
        }
    }

    return res;
}
