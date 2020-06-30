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

Tz TrainStation::timezone() const
{
    return timezoneForLocation(coordinate.latitude, coordinate.longitude, country);
}

SncfStationId::SncfStationId(const QString& id)
{
    if (id.size() != 5) {
        return;
    }
    setValue(fromChars(id.toUpper().toLatin1().constData()));
}

VRStationCode::VRStationCode(const QString &id)
{
    if (id.size() < 2 || id.size() > 4) {
        return;
    }
    char buffer[4];
    memset(buffer, 0, 4);
    memcpy(buffer, id.toUpper().toUtf8().constData(), id.size());
    setValue(fromChars(buffer));
}

TrainStation KnowledgeDb::stationForIbnr(IBNR ibnr)
{
    const auto ibnrIt = std::lower_bound(std::begin(ibnr_table), std::end(ibnr_table), ibnr);
    if (ibnrIt == std::end(ibnr_table) || (*ibnrIt).stationId != ibnr) {
        return {};
    }

    return trainstation_table[(*ibnrIt).stationIndex.value()];
}

TrainStation KnowledgeDb::stationForUic(UICStation uic)
{
    const auto it = std::lower_bound(std::begin(uic_table), std::end(uic_table), uic);
    if (it == std::end(uic_table) || (*it).stationId != uic) {
        return {};
    }
    return trainstation_table[(*it).stationIndex.value()];
}

TrainStation KnowledgeDb::stationForSncfStationId(SncfStationId sncfId)
{
    const auto it = std::lower_bound(std::begin(sncfStationId_table), std::end(sncfStationId_table), sncfId);
    if (it == std::end(sncfStationId_table) || (*it).stationId != sncfId) {
        return {};
    }

    return trainstation_table[(*it).stationIndex.value()];
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
    const auto it = std::lower_bound(std::begin(vrfiConnexionsId_table), std::end(vrfiConnexionsId_table), vrStation);
    if (it == std::end(vrfiConnexionsId_table) || (*it).stationId != vrStation) {
        return {};
    }

    return trainstation_table[(*it).stationIndex.value()];
}
