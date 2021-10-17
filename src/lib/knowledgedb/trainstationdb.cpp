/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "trainstationdb.h"
#include "trainstationdb_data.cpp"

#include <QString>

#include <algorithm>
#include <cstring>

using namespace KItinerary;
using namespace KItinerary::KnowledgeDb;

namespace KItinerary {
namespace KnowledgeDb {

static const auto trainstation_table_size = sizeof(trainstation_table) / sizeof(TrainStation);
static_assert(trainstation_table_size < (1 << (sizeof(TrainStationIndex) * 8)), "TrainStationIndex data type too small!");

}
}

template <typename Id, std::size_t Size>
static TrainStation lookupStation(Id id, const TrainStationIdIndex<Id>(&tab)[Size])
{
    const auto it = std::lower_bound(std::begin(tab), std::end(tab), id);
    if (it == std::end(tab) || (*it).stationId != id) {
        return {};
    }

    return trainstation_table[(*it).stationIndex.value()];
}

TrainStation KnowledgeDb::stationForIbnr(IBNR ibnr)
{
    return lookupStation(ibnr, ibnr_table);
}

TrainStation KnowledgeDb::stationForUic(UICStation uic)
{
    return lookupStation(uic, uic_table);
}

TrainStation KnowledgeDb::stationForSncfStationId(SncfStationId sncfId)
{
    return lookupStation(sncfId, sncfStationId_table);
}

TrainStation KnowledgeDb::stationForIndianRailwaysStationCode(const QString &code)
{
    const auto codeStr = code.toUtf8();
    const auto it = std::lower_bound(std::begin(indianRailwaysSationCode_index), std::end(indianRailwaysSationCode_index), codeStr, [](auto lhs, const QByteArray &rhs) {
        return strcmp(indianRailwaysSationCode_stringtable + lhs.offset, rhs.constData()) < 0;
    });
    if (it == std::end(indianRailwaysSationCode_index) || strcmp(indianRailwaysSationCode_stringtable + (*it).offset, codeStr.constData()) != 0) {
        return {};
    }

    return trainstation_table[(*it).stationIndex.value()];
}

TrainStation KnowledgeDb::stationForVRStationCode(VRStationCode vrStation)
{
    return lookupStation(vrStation, vrfiConnexionsId_table);
}

TrainStation KnowledgeDb::stationForBenerailId(BenerailStationId id)
{
    return lookupStation(id, benerail_table);
}
