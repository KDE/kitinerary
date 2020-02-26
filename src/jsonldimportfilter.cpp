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
#include "logging.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>

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
    { "BusinessEvent", "Event" },
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
            ticket.insert(QStringLiteral("@type"), QLatin1String("Ticket"));
        }
        if (!ticket.contains(QLatin1String("ticketToken"))) {
            ticket.insert(QStringLiteral("ticketToken"), token);
            res.insert(QStringLiteral("reservedTicket"), ticket);
            res.remove(QStringLiteral("ticketToken"));
        }
    }

    // unpack reservationFor array - if we ever encounter more than one element in here we'd need to multiply the result
    const auto resFor = res.value(QLatin1String("reservationFor"));
    if (resFor.isArray()) {
        const auto a = resFor.toArray();
        if (a.size() > 1) {
            qCWarning(Log) << "Found reservationFor array with" << a.size() << "elements!";
        }
        if (!a.isEmpty()) {
            res.insert(QStringLiteral("reservationFor"), a.at(0));
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

    // technically the wrong way (reservationId is the current schema.org standard), but hardly used anywhere (yet)
    renameProperty(res, "reservationId", "reservationNumber");

    // "typos"
    renameProperty(res, "Url", "url");
}

static void filterBusTrip(QJsonObject &trip)
{
    renameProperty(trip, "arrivalStation", "arrivalBusStop");
    renameProperty(trip, "departureStation", "departureBusStop");
    renameProperty(trip, "busCompany", "provider");
}

static void filterFoodEstablishment(QJsonObject &restaurant)
{
    // This can be a bool, "Yes"/"No", or a URL.
    auto reservationsValue = restaurant.value(QLatin1String("acceptsReservations"));
    if (reservationsValue.isString()) {
        const QString reservations = reservationsValue.toString();
        if (reservations == QLatin1String("Yes")) {
            restaurant.insert(QLatin1String("acceptsReservations"), true);
        } else if (reservations == QLatin1String("No")) {
            restaurant.insert(QLatin1String("acceptsReservations"), false);
        } else {
            migrateToAction(restaurant, "acceptsReservations", "ReserveAction", true);
        }
    }
}

static void filterActionTarget(QJsonObject &action)
{
    QJsonArray targets;
    QString filteredTargetUrlString;

    const QJsonValue oldTarget = action.value(QLatin1String("target"));
    if (oldTarget.isArray()) {
        targets = oldTarget.toArray();
    } else if (oldTarget.isObject()) {
        targets.push_back(oldTarget);
    }

    for (auto it = targets.begin(); it != targets.end(); ++it) {
        auto target = (*it).toObject();

        QJsonArray platforms;

        const QJsonValue actionPlatform = target.value(QLatin1String("actionPlatform"));
        if (actionPlatform.isArray()) {
            platforms = actionPlatform.toArray();
        } else {
            platforms.push_back(actionPlatform);
        }

        // Always return at least one URL but prefer the current platform if possible
        if (!filteredTargetUrlString.isEmpty()) {
            const bool hasPreferredPlatform = std::any_of(platforms.begin(), platforms.end(), [](const QJsonValue &platformValue) {
                const QString platform = platformValue.toString();
                // FIXME android
                return platform == QLatin1String("http://schema.org/DesktopWebPlatform");
            });

            if (!hasPreferredPlatform) {
                continue;
            }
        }

        const QUrl url(target.value(QLatin1String("urlTemplate")).toString());
        // It could also be a "URL template"
        if (!url.isValid()) {
            continue;
        }

        filteredTargetUrlString = url.toString();
    }

    if (filteredTargetUrlString.isEmpty()) {
        renameProperty(action, "url", "target");
    } else {
        action.insert(QStringLiteral("target"), filteredTargetUrlString);
    }
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
        filterActionTarget(action);
        *it = action;
    }

    return actions;
}


// filter functions applied to objects of the corresponding (already normalized) type
// IMPORTANT: keep alphabetically sorted by type!
static const struct {
    const char* type;
    void(*filterFunc)(QJsonObject&);
} type_filters[] = {
    { "BusTrip", filterBusTrip },
    { "Flight", filterFlight },
    { "FoodEstablishment", filterFoodEstablishment },
    { "LodgingReservation", filterLodgingReservation },
    { "TrainTrip", filterTrainTrip },
};

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
    auto type = obj.value(QLatin1String("@type")).toString().toUtf8();

    // normalize type
    const auto it = std::lower_bound(std::begin(type_mapping), std::end(type_mapping), type, [](const auto &lhs, const auto &rhs) {
        return std::strcmp(lhs.fromType, rhs.constData()) < 0;
    });
    if (it != std::end(type_mapping) && std::strcmp((*it).fromType, type.constData()) == 0) {
        type = it->toType;
        obj.insert(QStringLiteral("@type"), QLatin1String(type));
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

    // apply filter functions
    const auto filterIt = std::lower_bound(std::begin(type_filters), std::end(type_filters), type, [](const auto &lhs, const auto &rhs) {
        return std::strcmp(lhs.type, rhs.constData()) < 0;
    });
    if (filterIt != std::end(type_filters) && std::strcmp((*filterIt).type, type.constData()) == 0) {
        (*filterIt).filterFunc(obj);
    }
}

QJsonArray JsonLdImportFilter::filterObject(const QJsonObject& obj)
{
    QStringList types;
    const auto typeVal = obj.value(QLatin1String("@type"));
    if (typeVal.isString()) {
        types.push_back(typeVal.toString());
    } else if (typeVal.isArray()) {
        const auto typeNames = typeVal.toArray();
        for (const auto &t : typeNames) {
            if (t.isString()) {
                types.push_back(t.toString());
            }
        }
    }
    // TODO consider additionalTypes property

    QJsonArray results;

    for (const auto &type : types) {
        QJsonObject res(obj);
        res.insert(QStringLiteral("@type"), type);
        filterRecursive(res);

        if (type.endsWith(QLatin1String("Reservation"))) {
            filterReservation(res);
        }

        auto actions = res.value(QLatin1String("potentialAction"));
        if (!actions.isUndefined()) {
            res.insert(QStringLiteral("potentialAction"), filterActions(actions));
        }

        auto image = res.value(QLatin1String("image"));
        if (image.isArray()) {
            res.insert(QStringLiteral("image"), image.toArray().first());
        }

        image = res.value(QLatin1String("image"));
        if (image.isObject()) {
            const auto imageObject = image.toObject();
            if (imageObject.value(QLatin1String("@type")).toString() == QLatin1String("ImageObject")) {
                res.insert(QStringLiteral("image"), imageObject.value(QLatin1String("url")));
            }
        }

        results.push_back(res);
    }

    return results;
}
