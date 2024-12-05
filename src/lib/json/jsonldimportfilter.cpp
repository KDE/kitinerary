/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "jsonldimportfilter.h"
#include "json/jsonld.h"
#include "json/jsonldfilterengine.h"
#include "logging.h"

#include <QDate>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>

#include <cstring>

using namespace Qt::Literals::StringLiterals;
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

static void unpackArray(QJsonObject &obj, QLatin1StringView key) {
  const auto val = obj.value(key);
  if (!val.isArray()) {
    return;
  }
  const auto arr = val.toArray();
  if (arr.isEmpty()) {
    return;
  }
  obj.insert(key, arr.at(0));
}

static void migrateToAction(QJsonObject &obj, const char *propName, const char *typeName, bool remove)
{
  const auto value = obj.value(QLatin1StringView(propName));
  if (value.isNull() || value.isUndefined()) {
    return;
    }

    const auto actionsVal = obj.value("potentialAction"_L1);
    QJsonArray actions;
    if (actionsVal.isArray()) {
        actions = actionsVal.toArray();
    } else if (actionsVal.isObject()) {
        actions = { actionsVal };
    }

    for (const auto &act : actions) {
      if (JsonLd::typeName(act.toObject()) == QLatin1StringView(typeName)) {
        return;
      }
    }

    QJsonObject action;
    action.insert(QStringLiteral("@type"), QLatin1StringView(typeName));
    action.insert(QStringLiteral("target"), value);
    actions.push_back(action);
    obj.insert(QStringLiteral("potentialAction"), actions);

    if (remove) {
      obj.remove(QLatin1StringView(propName));
    }
}

static void filterPlace(QJsonObject &obj)
{
    // convert text address to PostalAddress
    if (const auto addr = obj.value("address"_L1); addr.isString()) {
        obj.insert("address"_L1, QJsonObject{
            {"@type"_L1, "PostalAddress"_L1},
            {"streetAddress"_L1, addr.toString()},
        });
    }
    // same for geo coordinates
    const auto lat = obj.value("latitude"_L1);
    const auto lon = obj.value("longitude"_L1);
    if (lat.isDouble() && lon.isDouble()) {
        auto geo = obj.value("geo"_L1).toObject();
        if (!geo.contains("@type"_L1)) {
            geo.insert("@type"_L1, "GeoCoordinates"_L1);
        }
        if (!geo.contains("latitude"_L1)) {
            geo.insert("latitude"_L1, lat);
        }
        if (!geo.contains("longitude"_L1)) {
            geo.insert("longitude"_L1, lon);
        }
        obj.insert("geo"_L1, geo);
    }
}

static void filterFlight(QJsonObject &res)
{
    // move incomplete departureTime (ie. just ISO date, no time) to departureDay
    if (res.value(QLatin1StringView("departureTime")).toString().size() == 10) {
      JsonLd::renameProperty(res, "departureTime", "departureDay");
    }
}

static void filterReservation(QJsonObject &res)
{
    // move ticketToken and ticketNumber to Ticket (Google vs. schema.org difference)
    for (const auto key : {"ticketToken"_L1, "ticketNumber"_L1}) {
        const auto v = res.value(key).toString();
        if (!v.isEmpty()) {
            auto ticket = res.value("reservedTicket"_L1).toObject();
            if (ticket.isEmpty()) {
                ticket.insert("@type"_L1, u"Ticket"_s);
            }
            if (!ticket.contains(key)) {
                ticket.insert(key, v);
                res.insert("reservedTicket"_L1, ticket);
                res.remove(key);
            }
        }
    }

    // normalize reservationStatus enum
    auto resStat = res.value("reservationStatus"_L1).toString();
    if (!resStat.isEmpty()) {
        if (resStat.startsWith("https:"_L1)) {
            resStat.remove(4, 1);
        }
        if (!resStat.contains("/Reservation"_L1)) {
            res.insert("reservationStatus"_L1, resStat.replace("http://schema.org/"_L1, "http://schema.org/Reservation"_L1));
        }
    }

    // legacy properties
    JsonLd::renameProperty(res, "programMembership", "programMembershipUsed");
    JsonLd::renameProperty(res, "price", "totalPrice");

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
    auto reservationsValue =
        restaurant.value(QLatin1StringView("acceptsReservations"));
    if (reservationsValue.isString()) {
        const QString reservations = reservationsValue.toString();
        if (reservations == QLatin1StringView("Yes")) {
          restaurant.insert(QLatin1StringView("acceptsReservations"), true);
        } else if (reservations == QLatin1StringView("No")) {
          restaurant.insert(QLatin1StringView("acceptsReservations"), false);
        } else {
          migrateToAction(restaurant, "acceptsReservations", "ReserveAction",
                          true);
        }
    }

    filterPlace(restaurant);
}

static void filterActionTarget(QJsonObject &action)
{
    QJsonArray targets;
    QString filteredTargetUrlString;

    const QJsonValue oldTarget = action.value(QLatin1StringView("target"));
    if (oldTarget.isArray()) {
        targets = oldTarget.toArray();
    } else if (oldTarget.isObject()) {
        targets.push_back(oldTarget);
    }

    for (auto it = targets.begin(); it != targets.end(); ++it) {
        auto target = (*it).toObject();

        QJsonArray platforms;

        const QJsonValue actionPlatform =
            target.value(QLatin1StringView("actionPlatform"));
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
                return platform == QLatin1StringView(
                                       "http://schema.org/DesktopWebPlatform");
            });

            if (!hasPreferredPlatform) {
                continue;
            }
        }

        const QUrl url(
            target.value(QLatin1StringView("urlTemplate")).toString());
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

static void filterEvent(QJsonObject &obj)
{
    unpackArray(obj, "location"_L1);

    // date only end: set time to end of day
    if (const auto endDate = obj.value("endDate"_L1).toString(); endDate.size() == 10) {
        const auto date = QDate::fromString(endDate, Qt::ISODate);
        if (date.isValid()) {
            obj.insert("endDate"_L1, date.endOfDay().toString(Qt::ISODate));
        }
    }
}

static void filterPostalAddress(QJsonObject &obj)
{
    // unpack country objects
    auto country = obj.value(QLatin1StringView("addressCountry"));
    if (country.isObject()) {
      obj.insert(QLatin1StringView("addressCountry"),
                 country.toObject().value(QLatin1StringView("name")));
    }
}

// filter functions applied to objects of the corresponding (already normalized) type
// IMPORTANT: keep alphabetically sorted by type!
static constexpr const JsonLdFilterEngine::TypeFilter type_filters[] = {
    { "Event", filterEvent },
    { "Flight", filterFlight },
    { "FoodEstablishment", filterFoodEstablishment },
    { "LocalBusiness", filterPlace },
    { "LodgingBusiness", filterPlace },
    { "Organization", filterPlace },
    { "Place", filterPlace },
    { "PostalAddress", filterPostalAddress },
};

// property renaming
// IMPORTANT: keep alphabetically sorted by type!
static constexpr const JsonLdFilterEngine::PropertyMapping property_mappings[] = {
    { "BusTrip", "arrivalStation", "arrivalBusStop" },
    { "BusTrip", "busCompany", "provider" },
    { "BusTrip", "departureStation", "departureBusStop" },

    // technically the wrong way around, but we still use the much more common old name
    { "Flight", "provider", "airline" },

    // check[in|out]Date -> check[in|out]Time (legacy Google format)
    { "LodgingReservation", "checkinDate", "checkinTime" },
    { "LodgingReservation", "checkoutDate", "checkoutTime" },

    { "ProgramMembership", "program", "programName" },
    { "ProgramMembership", "memberNumber", "membershipNumber" },

    { "Reservation", "price", "totalPrice" },
    { "Ticket", "price", "totalPrice" },

    // move TrainTrip::trainCompany to TrainTrip::provider (as defined by schema.org)
    { "TrainTrip", "trainCompany", "provider" },
};

static QJsonArray graphExpand(const QJsonObject &obj)
{
    QJsonArray result;

    const auto graph = obj.value(QLatin1StringView("@graph")).toArray();
    for (const auto &o : graph) {
        const auto a = JsonLdImportFilter::filterObject(o.toObject());
        std::copy(a.begin(), a.end(), std::back_inserter(result));
    }

    return result;
}

QJsonArray JsonLdImportFilter::filterObject(const QJsonObject &obj)
{
    QStringList types;
    const auto typeVal = obj.value(QLatin1StringView("@type"));
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

        // fold mainEntityOfPage into res
        if (const auto mainEntityOfPage =
                res.value(QLatin1StringView("mainEntityOfPage")).toObject();
            !mainEntityOfPage.isEmpty()) {
          res.remove(QLatin1StringView("mainEntityOfPage"));
          for (auto it = mainEntityOfPage.begin(); it != mainEntityOfPage.end();
               ++it) {
            if (it.key().startsWith(QLatin1Char('@')) ||
                res.contains(it.key())) {
              continue;
            }
            res.insert(it.key(), it.value());
          }
        }

        if (type.endsWith(QLatin1StringView("Reservation"))) {
          filterReservation(res);
        }

        auto actions = res.value(QLatin1StringView("potentialAction"));
        if (!actions.isUndefined()) {
            res.insert(QStringLiteral("potentialAction"), filterActions(actions));
        }

        unpackArray(res, QLatin1StringView("image"));
        const auto image = res.value(QLatin1StringView("image"));
        if (image.isObject()) {
            const auto imageObject = image.toObject();
            if (JsonLd::typeName(imageObject) ==
                QLatin1StringView("ImageObject")) {
              res.insert(QStringLiteral("image"),
                         imageObject.value(QLatin1StringView("url")));
            }
        }

        // unpack reservationFor array - multiply the result for each entry in here
        const auto resFor = res.value(QLatin1StringView("reservationFor"));
        if (const auto a = resFor.toArray(); !a.isEmpty()) {
            for (const auto &entry : a) {
              res.insert(QLatin1StringView("reservationFor"), entry);
              results.push_back(res);
            }
        } else {
            results.push_back(res);
        }
    }

    return results;
}
