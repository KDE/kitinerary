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

#include "trainstationdb.h"
#include "trainstationdb_data.cpp"

#include <QString>

#include <algorithm>

using namespace KItinerary;
using namespace KItinerary::KnowledgeDb;

namespace KItinerary {
namespace KnowledgeDb {

static const auto trainstation_table_size = sizeof(trainstation_table) / sizeof(TrainStation);
static_assert(trainstation_table_size < (1 << (sizeof(TrainStationIndex) * 8)), "TrainStationIndex data type too small!");

static const auto ibnr_table_size = sizeof(ibnr_table) / sizeof(IBNR);
static const IBNR* ibnrTableBegin() { return ibnr_table; }
static const IBNR* ibnrTableEnd() { return ibnr_table + ibnr_table_size; }

static const auto garesConnexionsId_table_size = sizeof(garesConnexionsId_table) / sizeof(GaresConnexionsId);
static const GaresConnexionsId* garesConnexionsIdBegin() { return garesConnexionsId_table; }
static const GaresConnexionsId* garesConnexionsIdEnd() { return garesConnexionsId_table + garesConnexionsId_table_size; }

}
}

GaresConnexionsId::GaresConnexionsId(const QString& id)
{
    if (id.size() != 5) {
        return;
    }
    m_id = fromChars(id.toUpper().toLatin1().constData());
}

TrainStation TrainStationDb::stationForIbnr(IBNR ibnr)
{
    const auto ibnrIt = std::lower_bound(ibnrTableBegin(), ibnrTableEnd(), ibnr);
    if (ibnrIt == ibnrTableEnd() || *ibnrIt != ibnr) {
        return {};
    }

    return trainstation_table[ibnr_index[std::distance(ibnrTableBegin(), ibnrIt)]];
}

TrainStation TrainStationDb::stationForGaresConnexionsId(GaresConnexionsId garesConnexionsId)
{
    const auto gcIt = std::lower_bound(garesConnexionsIdBegin(), garesConnexionsIdEnd(), garesConnexionsId);
    if (gcIt == garesConnexionsIdEnd() || *gcIt != garesConnexionsId) {
        return {};
    }

    return trainstation_table[garesConnexionsId_index[std::distance(garesConnexionsIdBegin(), gcIt)]];
}
