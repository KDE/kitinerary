/*
   Copyright (c) 2018 Volker Krause <vkrause@kde.org>

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   This library is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
   License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.
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

// geo coordinate to timezone index entry
struct TimezoneZIndexEntry {
    uint32_t z: 22;
    Tz tz: 9;
    bool isAmbiguous: 1;
};

static_assert(sizeof(TimezoneZIndexEntry) == 4, "structure is size-sensitive");

inline constexpr bool operator<(uint32_t lhs, TimezoneZIndexEntry rhs)
{
    return lhs < rhs.z;
}

}
}

#endif
