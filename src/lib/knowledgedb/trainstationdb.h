/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kitinerary_export.h"

#include "countrydb.h"
#include "iatacode.h"
#include "knowledgedb.h"
#include "stationidentifier.h"

#include <cstdint>

class QString;

namespace KItinerary {

namespace KnowledgeDb {

/** Position in the train station database. */
class TrainStationIndex : public UnalignedNumber<2> {
    using UnalignedNumber<2>::UnalignedNumber;
};

/** Train station id to station index lookup table structure. */
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

/** Lookup train station data by IATA location code. */
KITINERARY_EXPORT TrainStation stationForIataCode(IataCode iataCode);

/** Lookup train station data by Amtrak station code. */
KITINERARY_EXPORT TrainStation stationForAmtrakStationCode(AmtrakStationCode code);

/** Lookup train station data by Via Rail station code. */
KITINERARY_EXPORT TrainStation stationForViaRailStationCode(ViaRailStationCode code);

/** Lookup train station data by UK railway station code. */
KITINERARY_EXPORT TrainStation stationForUkRailwayStationCode(UKRailwayStationCode code);
}
}

