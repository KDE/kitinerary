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

}
}

GaresConnexionsId::GaresConnexionsId(const QString& id)
{
    if (id.size() != 5) {
        return;
    }
    setValue(fromChars(id.toUpper().toLatin1().constData()));
}

TrainStation KnowledgeDb::stationForIbnr(IBNR ibnr)
{
    const auto ibnrIt = std::lower_bound(std::begin(ibnr_table), std::end(ibnr_table), ibnr);
    if (ibnrIt == std::end(ibnr_table) || *ibnrIt != ibnr) {
        return {Coordinate{}, Timezone{}, CountryId{}};
    }

    return trainstation_table[ibnr_index[std::distance(std::begin(ibnr_table), ibnrIt)]];
}

TrainStation KnowledgeDb::stationForGaresConnexionsId(GaresConnexionsId garesConnexionsId)
{
    const auto gcIt = std::lower_bound(std::begin(garesConnexionsId_table), std::end(garesConnexionsId_table), garesConnexionsId);
    if (gcIt == std::end(garesConnexionsId_table) || *gcIt != garesConnexionsId) {
        return {Coordinate{}, Timezone{}, CountryId{}};
    }

    return trainstation_table[garesConnexionsId_index[std::distance(std::begin(garesConnexionsId_table), gcIt)]];
}
