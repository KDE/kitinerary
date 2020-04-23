/*
    Copyright (C) 2020 Volker Krause <vkrause@kde.org>

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

#ifndef OSM_GEOMATH_H
#define OSM_GEOMATH_H

#include "datatypes.h"

namespace OSM {

/** Distance between two coordinates. */
uint32_t distance(double lat1, double lon1, double lat2, double lon2);

/** Distance between @p coord1 and @p coord2 in meter. */
uint32_t distance(Coordinate coord1, Coordinate coord2);

/** Distance between the given polygon and coordinate, in meter. */
uint32_t distance(const std::vector<const OSM::Node*> &path, Coordinate coord);

}

#endif // OSM_GEOMATH_H
