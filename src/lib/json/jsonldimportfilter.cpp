/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "jsonldimportfilter.h"
#include "json/jsonldfilterengine.h"
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
static constexpr const JsonLdFilterEngine::TypeMapping type_mapping[] = {
    { "AutoDealer", "LocalBusiness" },
    { "AutoRepair", "LocalBusiness" },
    { "AutomotiveBusiness", "LocalBusiness" },
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
    { "ComputerStore", "LocalBusiness" },
    { "DanceEvent", "Event" },
    { "Distillery", "FoodEstablishment" },
    { "EditAction", "UpdateAction" },
    { "EducationEvent", "Event" },
    { "ElectronicsStore", "LocalBusiness" },
    { "EntertainmentBusiness", "LocalBusiness" },
    { "ExhibitionEvent", "Event" },
    { "FastFoodRestaurant", "FoodEstablishment" },
    { "Festival", "Event" },
    { "HobbyShop", "LocalBusiness" },
    { "HomeAndConstructionBusiness", "LocalBusiness" },
    { "Hostel", "LodgingBusiness" },
    { "Hotel", "LodgingBusiness" },
    { "IceCreamShop", "FoodEstablishment" },
    { "LiteraryEvent", "Event" },
    { "Motel", "LodgingBusiness" },
    { "MovieTheater", "LocalBusiness" },
    { "MusicEvent", "Event" },
    { "Resort", "LodgingBusiness" },
    { "Restaurant", "FoodEstablishment" },
    { "SaleEvent", "Event" },
    { "ScreeningEvent", "Event" },
    { "SocialEvent", "Event" },
    { "SportsEvent", "Event" },
    { "Store", "LocalBusiness" },
    { "TheaterEvent", "Event" },
    { "VisualArtsEvent", "Event" },
    { "Winery", "FoodEstablishment" },
};

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
        obj.remove(QLatin1String(propName));
    }
}

static void filterFlight(QJsonObject &res)
{
    // move incomplete departureTime (ie. just ISO date, no time) to departureDay
    if (res.value(QLatin1String("departureTime")).toString().size() == 10) {
        JsonLd::renameProperty(res, "departureTime", "departureDay");
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

    // normalize reservationStatus enum
    auto resStat = res.value(QLatin1String("reservationStatus")).toString();
    if (!resStat.isEmpty() && !resStat.contains(QLatin1String("/Reservation"))) {
        res.insert(QStringLiteral("reservationStatus"), resStat.replace(QLatin1String("http://schema.org/"), QLatin1String("http://schema.org/Reservation")));
    }

    // legacy properties
    JsonLd::renameProperty(res, "programMembership", "programMembershipUsed");

    // legacy potentialAction property
    JsonLd::renameProperty(res, "action", "potentialAction");

    // move Google xxxUrl properties to Action instances
    migrateToAction(res, "cancelReservationUrl", "CancelAction", true);
    migrateToAction(res, "checkinUrl", "CheckInAction", true);
    migrateToAction(res, "modifyReservationUrl", "UpdateAction", true);
    migrateToAction(res, "ticketDownloadUrl", "DownloadAction", true);
    migrateToAction(res, "url", "ViewAction", false);

    // technically the wrong way (reservationId is the current schema.org standard), but hardly used anywhere (yet)
    JsonLd::renameProperty(res, "reservationId", "reservationNumber");

    // "typos"
    JsonLd::renameProperty(res, "Url", "url");
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
        JsonLd::renameProperty(action, "url", "target");
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
static constexpr const JsonLdFilterEngine::TypeFilter type_filters[] = {
    { "Flight", filterFlight },
    { "FoodEstablishment", filterFoodEstablishment },
};

// property renaming
// IMPORTANT: keep alphabetically sorted by type!
static constexpr const JsonLdFilterEngine::PropertyMapping property_mappings[] = {
    { "BusTrip", "arrivalStation", "arrivalBusStop" },
    { "BusTrip", "busCompany", "provider" },
    { "BusTrip", "departureStation", "departureBusStop" },

    // check[in|out]Date -> check[in|out]Time (legacy Google format)
    { "LodgingReservation", "checkinDate", "checkinTime" },
    { "LodgingReservation", "checkoutDate", "checkoutTime" },

    { "ProgramMembership", "program", "programName" },
    { "ProgramMembership", "memberNumber", "membershipNumber" },

    // move TrainTrip::trainCompany to TrainTrip::provider (as defined by schema.org)
    { "TrainTrip", "trainCompany", "provider" },
};

static QJsonArray graphExpand(const QJsonObject &obj)
{
    QJsonArray result;

    const auto graph = obj.value(QLatin1String("@graph")).toArray();
    for (const auto &o : graph) {
        const auto a = JsonLdImportFilter::filterObject(o.toObject());
        std::copy(a.begin(), a.end(), std::back_inserter(result));
    }

    return result;
}

QJsonArray JsonLdImportFilter::filterObject(const QJsonObject &obj)
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

    if (types.isEmpty()) {
        return graphExpand(obj);
    }

    QJsonArray results;

    JsonLdFilterEngine filterEngine;
    filterEngine.setTypeMappings(type_mapping);
    filterEngine.setTypeFilters(type_filters);
    filterEngine.setPropertyMappings(property_mappings);
    for (const auto &type : types) {
        QJsonObject res(obj);
        res.insert(QStringLiteral("@type"), type);
        filterEngine.filterRecursive(res);

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