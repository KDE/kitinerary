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

#include "timezonedb.h"
#include "timezonedb_p.h"
#include "timezonedb_data.cpp"
// #include "timezone_zindex.cpp"

#include <QTimeZone>

using namespace KItinerary;

const char* KnowledgeDb::tzId(KnowledgeDb::Tz tz)
{
    return timezone_names + timezone_names_offsets[static_cast<std::underlying_type<KnowledgeDb::Tz>::type>(tz)];
}

QTimeZone KnowledgeDb::toQTimeZone(Tz tz)
{
    if (tz == Tz::Undefined) {
        return {};
    }
    return QTimeZone(tzId(tz));
}

KnowledgeDb::Tz KnowledgeDb::timezoneForCountry(CountryId country)
{
    const auto it = std::lower_bound(std::begin(country_timezone_map), std::end(country_timezone_map), country);
    if (it != std::end(country_timezone_map) && (*it).country == country) {
        return (*it).timezone;
    }

    return Tz::Undefined;
}

#if 0
KnowledgeDb::Tz KnowledgeDb::timezoneForCoordinate(float lat, float lon)
{
    const uint32_t x = ((lon + 180.0) / 360.0) * (1 << timezone_index_zDepth);
    const uint32_t y = ((lat + 90.0) / 180.0) * (1 << timezone_index_zDepth);
    uint32_t z = 0;
    for (int i = timezone_index_zDepth - 1; i >= 0; --i) {
        z <<= 1;
        z += (y & (1 << i)) ? 1 : 0;
        z <<= 1;
        z += (x & (1 << i)) ? 1 : 0;
    }

    const auto it = std::upper_bound(std::begin(timezone_index), std::end(timezone_index), z);
    if (it == std::begin(timezone_index)) {
        return Tz::Undefined;
    }
    return (*std::prev(it)).tz;
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

KnowledgeDb::Tz KnowledgeDb::timezoneForLocation(float lat, float lon, CountryId country)
{
    const auto coordTz = timezoneForCoordinate(lat, lon);
    const auto countryTz = timezoneForCountry(country);
    if (coordTz == Tz::Undefined || coordTz == countryTz) {
        return countryTz;
    }
    if (countryTz == Tz::Undefined) {
        return coordTz;
    }

    // if both timezones are equivalent, the country-based one wins, otherwise we use the coordinate one
    return isEquivalentTimezone(toQTimeZone(coordTz), toQTimeZone(countryTz)) ? countryTz : coordTz;
}
#endif
