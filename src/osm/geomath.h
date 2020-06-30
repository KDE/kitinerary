/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
