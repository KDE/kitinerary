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

#ifndef KITINERARY_KNOWLEDGEDB_H
#define KITINERARY_KNOWLEDGEDB_H

#include <cmath>

namespace KItinerary {

/** Lookup functions, utilities and data types for the static knowledge database.
 *  The content accessible by this functions is extracted from Wikidata and compiled
 *  into this library.
 *  @note The types in this namespace match the binary storage structure and thus
 *  are not intented for use in binary compatible APIs.
 */
namespace KnowledgeDb
{

/** Geographical coordinate.
 *  This matches the binary data layout on disk, it's not intended
 *  for use in API.
 */
struct Coordinate {
    inline constexpr Coordinate()
        : longitude(NAN)
        , latitude(NAN)
    {
    }

    inline explicit constexpr Coordinate(float lng, float lat)
        : longitude(lng)
        , latitude(lat)
    {
    }

    inline bool isValid() const
    {
        return !std::isnan(latitude) && !std::isnan(longitude);
    }

    inline constexpr bool operator==(Coordinate other) const
    {
        return latitude == other.latitude && longitude == other.longitude;
    }

    float longitude;
    float latitude;
};

}

}

#endif // KITINERARY_KNOWLEDGEDB_H
