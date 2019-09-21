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
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "jsonldimportfilter.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <cstring>

using namespace KItinerary;

// type normalization from full schema.org type hierarchy to our simplified subset
// IMPORTANT: keep alphabetically sorted by fromType!
static const struct {
    const char* fromType;
    const char* toType;
} type_mapping[] = {
    { "Bakery", "FoodEstablishment" },
    { "BarOrPub", "FoodEstablishment" },
    { "BedAndBreakfast", "LodgingBusiness" },
    { "Brewery", "FoodEstablishment" },
    { "BusStop", "BusStation" },
    { "CafeOrCoffeeShop", "FoodEstablishment" },
    { "Campground", "LodgingBusiness" },
    { "ChildrensEvent", "Event" },
    { "ComedyEvent", "Event" },
    { "DanceEvent", "Event" },
    { "Distillery", "FoodEstablishment" },
    { "EditAction", "UpdateAction" },
    { "EducationEvent", "Event" },
    { "ExhibitionEvent", "Event" },
    { "FastFoodRestaurant", "FoodEstablishment" },
    { "Festival", "Event" },
    { "Hostel", "LodgingBusiness" },
    { "Hotel", "LodgingBusiness" },
    { "IceCreamShop", "FoodEstablishment" },
    { "LiteraryEvent", "Event" },
    { "Motel", "LodgingBusiness" },
    { "MusicEvent", "Event" },
    { "Resort", "LodgingBusiness" },
    { "Restaurant", "FoodEstablishment" },
    { "SaleEvent", "Event" },
    { "ScreeningEvent", "Event" },
    { "SocialEvent", "Event" },
    { "SportsEvent", "Event" },
    { "TheaterEvent", "Event" },
    { "VisualArtsEvent", "Event" },
    { "Winery", "FoodEstablishment" },
};

static void renameProperty(QJsonObject &obj, const char *oldName, const char *newName)
{
    const auto value = obj.value(QLatin1String(oldName));
    if (!value.isNull() && !obj.contains(QLatin1String(newName))) {
        obj.insert(QLatin1String(newName), value);
        obj.remove(QLatin1String(oldName));
    }
}

static void migrateToAction(QJsonObject &obj, const char *propName, const char *typeName, bool remove)
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
    obj.insert(QStringLiteral("potentialAction"), actions);

    if (remove) {
        obj.remove(QStringLiteral("propName"));
    }
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

static void filterTaxiReservation(QJsonObject &res)
{
    renameProperty(res, "reservationId", "reservationNumber");
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
            ticket.insert(QStringLiteral("@type"), QLatin1String("Ticket"));
        }
        if (!ticket.contains(QLatin1String("ticketToken"))) {
            ticket.insert(QStringLiteral("ticketToken"), token);
            res.insert(QStringLiteral("reservedTicket"), ticket);
            res.remove(QStringLiteral("ticketToken"));
        }
    }

    // legacy potentialAction property
    renameProperty(res, "action", "potentialAction");

    // move Google xxxUrl properties to Action instances
    migrateToAction(res, "cancelReservationUrl", "CancelAction", true);
    migrateToAction(res, "checkinUrl", "CheckInAction", true);
    migrateToAction(res, "modifyReservationUrl", "UpdateAction", true);
    migrateToAction(res, "ticketDownloadUrl", "DownloadAction", true);
    migrateToAction(res, "url", "ViewAction", false);

    // "typos"
    renameProperty(res, "Url", "url");
}

static void filterBusTrip(QJsonObject &trip)
{
    renameProperty(trip, "arrivalStation", "arrivalBusStop");
    renameProperty(trip, "departureStation", "departureBusStop");
    renameProperty(trip, "busCompany", "provider");
}

static void filterBusReservation(QJsonObject &res)
{
    QJsonObject trip = res.value(QLatin1String("reservationFor")).toObject();
    filterBusTrip(trip);
    res.insert(QStringLiteral("reservationFor"), trip);
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
        renameProperty(action, "url", "target");
        *it = action;
    }

    return actions;
}

static void filterRecursive(QJsonObject &obj);

static void filterRecursive(QJsonArray &array)
{
    for (auto it = array.begin(); it != array.end(); ++it) {
        if ((*it).type() == QJsonValue::Object) {
            QJsonObject subObj = (*it).toObject();
            filterRecursive(subObj);
            *it = subObj;
        } else if ((*it).type() == QJsonValue::Array) {
            QJsonArray array = (*it).toArray();
            filterRecursive(array);
            *it = array;
        }
    }
}

static void filterRecursive(QJsonObject &obj)
{
    const auto type = obj.value(QLatin1String("@type")).toString().toUtf8();
    const auto it = std::lower_bound(std::begin(type_mapping), std::end(type_mapping), type, [](const auto &lhs, const auto &rhs) {
        return std::strcmp(lhs.fromType, rhs.constData()) < 0;
    });
    if (it != std::end(type_mapping) && std::strcmp((*it).fromType, type.constData()) == 0) {
        obj.insert(QStringLiteral("@type"), QLatin1String((*it).toType));
    }

    for (auto it = obj.begin(); it != obj.end(); ++it) {
        if ((*it).type() == QJsonValue::Object) {
            QJsonObject subObj = (*it).toObject();
            filterRecursive(subObj);
            *it = subObj;
        } else if ((*it).type() == QJsonValue::Array) {
            QJsonArray array = (*it).toArray();
            filterRecursive(array);
            *it = array;
        }
    }
}

QJsonObject JsonLdImportFilter::filterObject(const QJsonObject& obj)
{
    QJsonObject res(obj);
    filterRecursive(res);

    const auto type = obj.value(QLatin1String("@type")).toString();
    if (type.endsWith(QLatin1String("Reservation"))) {
        filterReservation(res);
    }

    if (type == QLatin1String("TrainReservation")) {
        auto train = obj.value(QLatin1String("reservationFor")).toObject();
        filterTrainTrip(train);
        if (!train.isEmpty()) {
            res.insert(QStringLiteral("reservationFor"), train);
        }
    } else if (type == QLatin1String("LodgingReservation")) {
        filterLodgingReservation(res);
    } else if (type == QLatin1String("FlightReservation")) {
        auto flight = obj.value(QLatin1String("reservationFor")).toObject();
        filterFlight(flight);
        if (!flight.isEmpty()) {
            res.insert(QStringLiteral("reservationFor"), flight);
        }
    } else if (type == QLatin1String("TaxiReservation")) {
        filterTaxiReservation(res);
    } else if (type == QLatin1String("BusReservation")) {
        filterBusReservation(res);
    }

    auto actions = res.value(QLatin1String("potentialAction"));
    if (!actions.isUndefined()) {
        res.insert(QStringLiteral("potentialAction"), filterActions(actions));
    }

    return res;
}
