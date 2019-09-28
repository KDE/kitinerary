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
class TrainStationIndex : public UnalignedNumber<2> {
    using UnalignedNumber<2>::UnalignedNumber;
};

/** Train station id to station index lookup table strcuture. */
template <typename T>
struct TrainStationIdIndex {
    constexpr bool operator<(const TrainStationIdIndex &other) const
    {
        return stationId < other.stationId;
    }
    constexpr bool operator<(T otherId) const
    {
        return stationId < otherId;
    }

    T stationId;
    TrainStationIndex stationIndex;
};

/** Train station entry in the station table.
 *  @note This is not for use in APIs/ABIs, but matches the binary layout of the database.
 */
struct TrainStation {
    Coordinate coordinate;
    Timezone timezone;
    CountryId country;
};

/** IBNR station id.
 *  2 digits UIC country code, 5 digits station id.
 *  Same format as UICStation, but nevertheless different values.
 */
class IBNR : public UnalignedNumber<3> {
    using UnalignedNumber<3>::UnalignedNumber;
};

/** UIC station id.
 *  2 digits UIC country code, 5 digits station id.
 *  Same format as IBNR, but nevertheless different values.
 */
class UICStation : public UnalignedNumber<3> {
    using UnalignedNumber<3>::UnalignedNumber;
};

/** Gares & Connexion ID.
 *  2 letters ISO country code, 5 letters station id, expected to be in upper case.
 */
class GaresConnexionsId : public UnalignedNumber<3>
{
public:
    inline constexpr GaresConnexionsId() = default;
    inline explicit constexpr GaresConnexionsId(const char s[5])
        : UnalignedNumber<3>(fromChars(s))
    {
    }

    KITINERARY_EXPORT explicit GaresConnexionsId(const QString &id);

private:
    static inline constexpr uint32_t fromChars(const char s[5])
    {
        return (s[4] - '@') + 26 * ((s[3] - '@') + 26 * ((s[2] - '@') + 26 * ((s[1] - '@') + 26 * (s[0] - '@'))));
    }
};

/** Lookup train station data by IBNR. */
KITINERARY_EXPORT TrainStation stationForIbnr(IBNR ibnr);

/** Lookup train station data by UIC station id. */
KITINERARY_EXPORT TrainStation stationForUic(UICStation uic);

/** Lookup train station data by Gares & Connexions ID. */
KITINERARY_EXPORT TrainStation stationForGaresConnexionsId(GaresConnexionsId garesConnexionsId);

/** Lookup train station data by Indian Railways station code. */
KITINERARY_EXPORT TrainStation stationForIndianRailwaysStationCode(const QString &code);

}
}

#endif // KITINERARY_TRAINSTATIONDB_H
