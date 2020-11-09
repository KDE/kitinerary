/*
   SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_KNOWLEDGEDB_TIMEZONEDB_P_H
#define KITINERARY_KNOWLEDGEDB_TIMEZONEDB_P_H

#include "countrydb.h"
#include "timezonedb.h"

namespace KItinerary {
namespace KnowledgeDb  {

// country to timezone mapping (for countries where this is unique)
struct CountryTimezoneMap {
    CountryId country;
    Tz timezone;
};

inline constexpr bool operator<(const CountryTimezoneMap lhs, const CountryId rhs)
{
    return lhs.country < rhs;
}

// encoding parameters for the timezone index
struct TimezoneZIndexParams {
    float   xStart;
    float   xRange;
    float   yStart;
    float   yRange;
    uint8_t zDepth;

    constexpr inline float yEnd() const { return yStart + yRange; }
};

// geo coordinate to timezone index entry
// Note: MSVC cannot create bitfields across unsigned integer types and enums/bools (as those are signed there)
// so we need to use a single type for the bitfield and do the conversion manually (and very forcefully)
struct TimezoneZIndexEntry {
    inline constexpr TimezoneZIndexEntry(uint32_t _z, Tz _tz, bool _isAmbiguous)
        : z(_z)
        , tz(uint32_t(_tz))
        , isAmbiguous(_isAmbiguous)
    {}

    uint32_t z: 22;
    uint32_t tz: 9;
    uint32_t isAmbiguous: 1;
};

static_assert(sizeof(TimezoneZIndexEntry) == 4, "structure is size-sensitive");

inline constexpr bool operator<(uint32_t lhs, TimezoneZIndexEntry rhs)
{
    return lhs < rhs.z;
}

}
}

#endif
