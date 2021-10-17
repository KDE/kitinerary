/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "timezonedb.h"
#include "timezonedb_p.h"
#include "timezonedb_data.cpp"
#include "timezone_zindex.cpp"

#include <QTimeZone>

#include <cmath>

using namespace KItinerary;

const char* KnowledgeDb::tzId(KnowledgeDb::Tz tz)
{
    return timezone_names + timezone_names_offsets[static_cast<std::underlying_type<KnowledgeDb::Tz>::type>(tz)];
}

static QTimeZone toQTimeZone(KnowledgeDb::Tz tz)
{
    if (tz == KnowledgeDb::Tz::Undefined) {
        return {};
    }
    return QTimeZone(tzId(tz));
}

static KnowledgeDb::Tz timezoneForCountry(KnowledgeDb::CountryId country)
{
    using namespace KnowledgeDb;
    const auto it = std::lower_bound(std::begin(country_timezone_map), std::end(country_timezone_map), country);
    if (it != std::end(country_timezone_map) && (*it).country == country) {
        return (*it).timezone;
    }

    return Tz::Undefined;
}

KnowledgeDb::CountryId KnowledgeDb::countryForTimezone(KnowledgeDb::Tz tz)
{
    return timezone_country_map[static_cast<std::underlying_type<KnowledgeDb::Tz>::type>(tz)];
}

KnowledgeDb::Tz KnowledgeDb::timezoneForCoordinate(float lat, float lon, bool *ambiguous)
{
    if (std::isnan(lat) || std::isnan(lon)) {
        return Tz::Undefined;
    }

    // see arctic latitude filter in the generator script, we only cover 65°S to 80°N
    if (lat < timezone_index_params.yStart || lat > timezone_index_params.yEnd()) {
        return Tz::Undefined;
    }

    const uint32_t x = ((lon - timezone_index_params.xStart) / timezone_index_params.xRange) * (1 << timezone_index_params.zDepth);
    const uint32_t y = ((lat - timezone_index_params.yStart) / timezone_index_params.yRange) * (1 << timezone_index_params.zDepth);
    uint32_t z = 0;
    for (int i = timezone_index_params.zDepth - 1; i >= 0; --i) {
        z <<= 1;
        z += (y & (1 << i)) ? 1 : 0;
        z <<= 1;
        z += (x & (1 << i)) ? 1 : 0;
    }

    const auto it = std::upper_bound(std::begin(timezone_index), std::end(timezone_index), z);
    if (it == std::begin(timezone_index)) {
        return Tz::Undefined;
    }
    if (ambiguous) {
        *ambiguous = (*std::prev(it)).isAmbiguous;
    }
    return static_cast<Tz>((*std::prev(it)).tz);
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

QTimeZone KnowledgeDb::timezoneForLocation(float lat, float lon, CountryId country)
{
    bool ambiguous = false;
    const auto coordTz = timezoneForCoordinate(lat, lon, &ambiguous);
    const auto countryTz = timezoneForCountry(country);
    const auto countryFromCoord = countryForTimezone(coordTz);

    // if we determine a different country than was provided, search for an equivalent timezone
    // in the requested country
    // example: Tijuana airport ending up in America/Los Angeles, and America/Tijuna being the only MX timezone equivalent to that
    if (coordTz != Tz::Undefined && countryFromCoord.isValid() && countryFromCoord != country) {
        bool nonUnique = false;
        Tz foundTz = Tz::Undefined;
        const auto coordZone = toQTimeZone(coordTz);

        constexpr const int timezone_count = sizeof(timezone_country_map) / sizeof(timezone_country_map[0]);
        for (int i = 1; i < timezone_count; ++i) {
            if (timezone_country_map[i] != country) {
                continue;
            }
            const auto t = static_cast<Tz>(i);
            if (!isEquivalentTimezone(toQTimeZone(t), coordZone)) {
                continue;
            }
            if (foundTz != Tz::Undefined) {
                nonUnique = true;
                break;
            }
            foundTz = t;
        }

        if (!nonUnique && foundTz != Tz::Undefined) {
            return toQTimeZone(foundTz);
        }
    }

    // only one method found a result, let's use that one
    if (coordTz == Tz::Undefined || coordTz == countryTz) {
        return toQTimeZone(countryTz);
    }
    if (countryTz == Tz::Undefined) {
        return toQTimeZone(coordTz);
    }

    // if the coordinate-based timezone is also in @p country, that takes precedence
    // example: the various AR sub-zones, or the MY sub-zone
    if (country == countryFromCoord || !ambiguous) {
        return toQTimeZone(coordTz);
    }

    // if both timezones are equivalent, the country-based one wins, otherwise we use the coordinate one
    const auto coordQtz = toQTimeZone(coordTz);
    const auto countryQtz = toQTimeZone(countryTz);
    return isEquivalentTimezone(coordQtz, countryQtz) ? countryQtz : coordQtz;
}

KnowledgeDb::CountryId KnowledgeDb::countryForCoordinate(float lat, float lon)
{
    bool ambiguous = false;
    const auto tz = timezoneForCoordinate(lat, lon, &ambiguous);
    if (!ambiguous) {
        return countryForTimezone(tz);
    }
    return {};
}
