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

#include "geomath.h"

#include <QLineF>

#include <cmath>
#include <limits>

using namespace OSM;

// see https://en.wikipedia.org/wiki/Haversine_formula
uint32_t OSM::distance(double lat1, double lon1, double lat2, double lon2)
{
    const auto degToRad = M_PI / 180.0;
    const auto earthRadius = 6371000.0; // in meters

    const auto d_lat = (lat1 - lat2) * degToRad;
    const auto d_lon = (lon1 - lon2) * degToRad;

    const auto a = pow(sin(d_lat / 2.0), 2) + cos(lat1 * degToRad) * cos(lat2 * degToRad) * pow(sin(d_lon / 2.0), 2);
    return 2.0 * earthRadius * atan2(sqrt(a), sqrt(1.0 - a));}

uint32_t OSM::distance(Coordinate coord1, Coordinate coord2)
{
    return distance(coord1.latF(), coord1.lonF(), coord2.latF(), coord2.lonF());
}

uint32_t OSM::distance(const std::vector<const OSM::Node*> &path, OSM::Coordinate coord)
{
    if (path.empty()) {
        return std::numeric_limits<uint32_t>::max();
    }

    if (path.size() == 1) {
        return distance(path[0]->coordinate, coord);
    }

    uint32_t dist = std::numeric_limits<uint32_t>::max();
    OSM::Id firstNode = 0;
    for (auto it = path.begin(); it != std::prev(path.end()); ++it) {
        const auto nextIt = std::next(it);
        if (firstNode == 0) { // starting a new loop
            firstNode = (*it)->id;
        }
        if ((*nextIt)->id == firstNode) { // just closed a loop, so this is not a line on the path
            firstNode = 0;
            continue;
        }

        // compute distance between line segment and coord
        const QLineF lineSegment(QPointF((*it)->coordinate.latF(), (*it)->coordinate.lonF()), QPointF((*nextIt)->coordinate.latF(), (*nextIt)->coordinate.lonF()));
        QLineF n = lineSegment.normalVector();
        n.translate(coord.latF() - n.p1().x(), coord.lonF() - n.p1().y());
        QPointF p;
        const auto intersect = lineSegment.intersects(n, &p);
        if (intersect == QLineF::BoundedIntersection) {
            dist = std::min(dist, distance(p.x(), p.y(), coord.latF(), coord.lonF()));
        } else {
            dist = std::min(dist, std::min(distance((*it)->coordinate, coord), distance((*nextIt)->coordinate, coord)));
        }
    }
    return dist;
}
