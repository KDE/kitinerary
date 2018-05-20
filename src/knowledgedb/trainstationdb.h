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
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KITINERARY_TRAINSTATIONDB_H
#define KITINERARY_TRAINSTATIONDB_H

#include "kitinerary_export.h"
#include "countrydb.h"
#include "knowledgedb.h"
#include "timezonedb.h"

#include <cstdint>

class QString;

namespace KItinerary {

namespace KnowledgeDb {

/** Position in the train station database. */
typedef uint16_t TrainStationIndex;

/** Train station entry in the station table.
 *  @note This is not for use in APIs/ABIs, but matches the binary layout of the database.
 */
struct TrainStation {
    Coordinate coordinate;
    Timezone timezone;
    CountryId country;
};

/** IBNR station id.
 *  2 digits UIC country code, 5 digits station id
 */
struct IBNR {
    inline constexpr IBNR() = default;
    inline explicit constexpr IBNR(uint32_t ibnr)
        : id(ibnr)
    {}

    inline constexpr bool operator!=(IBNR other) const
    {
        return id != other.id;
    }
    inline constexpr bool operator<(IBNR other) const
    {
        return id < other.id;
    }

    uint32_t id = 0; // TODO 24bit would be enough
};

/** Gares & Connexion ID.
 *  2 letters ISO country code, 5 letters station id, expected to be in upper case.
 */
class GaresConnexionsId
{
public:
    inline constexpr GaresConnexionsId() = default;
    inline explicit constexpr GaresConnexionsId(const char s[5])
        : m_id(fromChars(s))
    {
    }

    KITINERARY_EXPORT explicit GaresConnexionsId(const QString &id);

    inline constexpr bool operator!=(GaresConnexionsId other) const
    {
        return m_id != other.m_id;
    }
    inline constexpr bool operator<(GaresConnexionsId other) const
    {
        return m_id < other.m_id;
    }

private:
    static inline constexpr uint32_t fromChars(const char s[5])
    {
        return (s[4] - '@') + 26 * ((s[3] - '@') + 26 * ((s[2] - '@') + 26 * ((s[1] - '@') + 26 * (s[0] - '@'))));
    }

    // TODO 24bit would be enough
    uint32_t m_id = 0;
};
}

/** Static train station data from Wikidata. */
namespace TrainStationDb
{
    /** Lookup station data by IBNR. */
    KITINERARY_EXPORT KnowledgeDb::TrainStation stationForIbnr(KnowledgeDb::IBNR ibnr);

    /** Lookup station data by Gares & Connexions ID. */
    KITINERARY_EXPORT KnowledgeDb::TrainStation stationForGaresConnexionsId(KnowledgeDb::GaresConnexionsId garesConnexionsId);
}

}

#endif // KITINERARY_TRAINSTATIONDB_H
