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

#include "datatypes.h"

using namespace OSM;

void DataSet::addNode(Node &&node)
{
    const auto it = std::lower_bound(nodes.begin(), nodes.end(), node);
    if (it != nodes.end() && (*it).id == node.id) {
        // do we need to merge something here?
        return;
    }
    nodes.insert(it, std::move(node));
}

void DataSet::addWay(Way &&way)
{
    const auto it = std::lower_bound(ways.begin(), ways.end(), way);
    if (it != ways.end() && (*it).id == way.id) {
        // already there?
        return;
    }
    ways.insert(it, std::move(way));
}

void DataSet::addRelation(Relation &&rel)
{
    const auto it = std::lower_bound(relations.begin(), relations.end(), rel);
    if (it != relations.end() && (*it).id == rel.id) {
        // do we need to merge something here?
        return;
    }
    relations.insert(it, std::move(rel));
}

QString OSM::Node::url() const
{
    return QStringLiteral("https://openstreetmap.org/node/") + QString::number(id);
}

bool OSM::Way::isClosed() const
{
    return nodes.size() >= 2 && nodes.front() == nodes.back();
}

QString OSM::Way::url() const
{
    return QStringLiteral("https://openstreetmap.org/way/") + QString::number(id);
}

QString OSM::Relation::url() const
{
    return QStringLiteral("https://openstreetmap.org/relation/") + QString::number(id);
}

QDebug operator<<(QDebug debug, OSM::Coordinate coord)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << '(' << coord.latF() << ',' << coord.lonF() << ')';
    return debug;
}

QDebug operator<<(QDebug debug, OSM::BoundingBox bbox)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << '[' << bbox.min.latF() << ',' << bbox.min.lonF() << '|' << bbox.max.latF() << ',' << bbox.max.lonF() << ']';
    return debug;
}
