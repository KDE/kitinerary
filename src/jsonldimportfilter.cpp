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

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

using namespace KItinerary;

static void renameProperty(QJsonObject &obj, const char *oldName, const char *newName)
{
    const auto value = obj.value(QLatin1String(oldName));
    if (!value.isNull() && !obj.contains(QLatin1String(newName))) {
        obj.insert(QLatin1String(newName), value);
        obj.remove(QLatin1String(oldName));
    }
}

static void moveToAction(QJsonObject &obj, const char *propName, const char *typeName)
{
    const auto value = obj.value(QLatin1String(propName));
    if (value.isNull() || value.isUndefined()) {
        return;
    }

    auto actions = obj.value(QLatin1String("potentialAction")).toArray();
    for (const auto &act : actions) {
        if (act.toObject().value(QLatin1String("@type")).toString() == QLatin1String(typeName)) {
            return;
        }
    }

    QJsonObject action;
    action.insert(QStringLiteral("@type"), QLatin1String(typeName));
    action.insert(QStringLiteral("target"), value);
    actions.push_back(action);
    obj.insert(QLatin1String("potentialAction"), actions);
}

static void filterTrainTrip(QJsonObject &trip)
{
    if (trip.value(QLatin1String("@type")).toString() != QLatin1String("TrainTrip")) {
        return;
    }

    // move TrainTrip::trainCompany to TrainTrip::provider (as defined by schema.org)
    renameProperty(trip, "trainCompany", "provider");
}

static void filterLodgingReservation(QJsonObject &res)
{
    // check[in|out]Date -> check[in|out]Time (legacy Google format)
    renameProperty(res, "checkinDate", "checkinTime");
    renameProperty(res, "checkoutDate", "checkoutTime");
}

static void filterFlight(QJsonObject &res)
{
    // move incomplete departureTime (ie. just ISO date, no time) to departureDay
    if (res.value(QLatin1String("departureTime")).toString().size() == 10) {
        renameProperty(res, "departureTime", "departureDay");
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
            res.remove(QLatin1String("ticketToken"));
        }
    }

    // move Google xxxUrl properties to Action instances
    moveToAction(res, "cancelReservationUrl", "CancelAction");
    moveToAction(res, "checkinUrl", "CheckInAction");
    moveToAction(res, "modifyReservationUrl", "UpdateAction");
    moveToAction(res, "ticketDownloadUrl", "DownloadAction");
    moveToAction(res, "url", "ViewAction");
}

static QJsonArray filterActions(const QJsonValue &v)
{
    QJsonArray actions;
    if (v.isArray()) {
        actions = v.toArray();
    } else {
        actions.push_back(v);
    }

    for (auto it = actions.begin(); it != actions.end(); ++it) {
        auto action = (*it).toObject();
        if (action.value(QLatin1String("@type")).toString() == QLatin1String("EditAction")) {
            action.insert(QLatin1String("@type"), QLatin1String("UpdateAction"));
        }
        *it = action;
    }

    return actions;
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
    } else if (type == QLatin1String("LodgingReservation")) {
        filterLodgingReservation(res);
    } else if (type == QLatin1String("FlightReservation")) {
        auto flight = obj.value(QLatin1String("reservationFor")).toObject();
        filterFlight(flight);
        if (!flight.isEmpty()) {
            res.insert(QLatin1String("reservationFor"), flight);
        }
    }

    auto actions = res.value(QLatin1String("potentialAction"));
    if (!actions.isUndefined()) {
        res.insert(QLatin1String("potentialAction"), filterActions(actions));
    }

    return res;
}
