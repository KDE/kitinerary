/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "config-kitinerary.h"

#include "timezonedb.h"

#include <KCountry>
#include <KTimeZone>

#include <QDebug>
#include <QTimeZone>

#include <cmath>
#include <cstring>

using namespace KItinerary;

static QTimeZone toQTimeZone(const char *tzId)
{
    if (!tzId || std::strcmp(tzId, "") == 0) {
        return {};
    }
    return QTimeZone(tzId);
}

static QList<const char*> timezonesForCountry(const KCountry &country)
{
    auto tzs = country.timeZoneIds();
    if (tzs.size() <= 1) {
        return tzs;
    }
    // filter overseas territories that map back to a different country code
    tzs.erase(std::remove_if(tzs.begin(), tzs.end(), [country](const char *tz) {
        return !(KTimeZone::country(tz) == country);
    }), tzs.end());
    return tzs;
}

static bool compareOffsetData(const QTimeZone::OffsetData &lhs, const QTimeZone::OffsetData &rhs)
{
    return lhs.offsetFromUtc == rhs.offsetFromUtc
        && lhs.standardTimeOffset == rhs.standardTimeOffset
        && lhs.daylightTimeOffset == rhs.daylightTimeOffset
        && lhs.atUtc == rhs.atUtc
        && lhs.abbreviation == rhs.abbreviation;
}

static bool isEquivalentTimezone(const QTimeZone &lhs, const QTimeZone &rhs)
{
    auto dt = QDateTime::currentDateTimeUtc();
    if (lhs.offsetFromUtc(dt) != rhs.offsetFromUtc(dt) || lhs.hasTransitions() != rhs.hasTransitions()) {
        return false;
    }

    for (int i = 0; i < 2 && dt.isValid(); ++i) {
        const auto lhsOff = lhs.nextTransition(dt);
        const auto rhsOff = rhs.nextTransition(dt);
        if (!compareOffsetData(lhsOff, rhsOff)) {
            return false;
        }
        dt = lhsOff.atUtc;
    }

    return true;
}

QTimeZone KnowledgeDb::timezoneForLocation(float lat, float lon, QStringView alpha2CountryCode, QStringView regionCode)
{

    const auto coordTz = KTimeZone::fromLocation(lat, lon);
    const auto coordZone = toQTimeZone(coordTz);

    const auto country = KCountry::fromAlpha2(alpha2CountryCode);
    auto countryTzs = KCountrySubdivision::fromCode(QString(alpha2CountryCode + QLatin1Char('-') + regionCode)).timeZoneIds();
    if (countryTzs.isEmpty()) {
        countryTzs = timezonesForCountry(country);
    }
    const auto countryFromTz = KTimeZone::country(coordTz);

    // if we determine a different country than was provided, search for an equivalent timezone
    // in the requested country
    // example: Tijuana airport ending up in America/Los Angeles, and America/Tijuna being the only MX timezone equivalent to that
    if (coordTz && countryFromTz.isValid() && country.isValid() && !(countryFromTz == country)) { // ### clean up once KCountry has op!=
        bool nonUnique = false;
        QTimeZone foundTz;

        for (const char *countryTz : countryTzs) {
            const auto t = toQTimeZone(countryTz);
            if (!isEquivalentTimezone(t, coordZone)) {
                continue;
            }
            if (foundTz.isValid()) {
                nonUnique = true;
                break;
            }
            foundTz = t;
        }

        if (!nonUnique && foundTz.isValid()) {
            return foundTz;
        }
    }

    // only one method found a result, let's use that one
    if ((coordZone.isValid() && countryTzs.contains(coordTz)) || countryTzs.isEmpty()) {
        return coordZone;
    }
    if (!coordZone.isValid() && countryTzs.size() == 1) {
        return toQTimeZone(countryTzs.at(0));
    }

    // if the coordinate-based timezone is also in @p country, that takes precedence
    // example: the various AR sub-zones, or the MY sub-zone
    if (country == countryFromTz) {
        return coordZone;
    }

    // if both timezones are equivalent, the country-based one wins, otherwise we use the coordinate one
    if (countryTzs.size() == 1) {
        const auto countryQtz = toQTimeZone(countryTzs.at(0));
        return isEquivalentTimezone(coordZone, countryQtz) ? countryQtz : coordZone;
    }
    return coordZone;
}
