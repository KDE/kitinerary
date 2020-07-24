/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_TRAINSTATIONDB_H
#define KITINERARY_TRAINSTATIONDB_H

#include "kitinerary_export.h"
#include "stationidentifier.h"
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
    CountryId country;

    KITINERARY_EXPORT Tz timezone() const;
};


/** Lookup train station data by IBNR. */
KITINERARY_EXPORT TrainStation stationForIbnr(IBNR ibnr);

/** Lookup train station data by UIC station id. */
KITINERARY_EXPORT TrainStation stationForUic(UICStation uic);

/** Lookup train station data by SNCF station id. */
KITINERARY_EXPORT TrainStation stationForSncfStationId(SncfStationId sncfId);

/** Lookup train station data by Indian Railways station code. */
KITINERARY_EXPORT TrainStation stationForIndianRailwaysStationCode(const QString &code);

/** Lookup train station data by VR (Finland) station code. */
KITINERARY_EXPORT TrainStation stationForVRStationCode(VRStationCode vrStation);

/** Lookup train station data by Benerail station identifier. */
KITINERARY_EXPORT TrainStation stationForBenerailId(BenerailStationId id);

}
}

#endif // KITINERARY_TRAINSTATIONDB_H
