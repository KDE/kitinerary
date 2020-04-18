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

#ifndef OSMAIRPORTDB_H
#define OSMAIRPORTDB_H

#include <osm/datatypes.h>

#include <QPolygonF>

#include <map>
#include <vector>

struct OSMAirportData
{
    QString source; // OSM URL, for diagnostics only
    OSM::BoundingBox bbox;

    std::vector<const OSM::Way*> airportPaths;
    QPolygonF airportPolygon;

    std::vector<OSM::BoundingBox> terminalBboxes;
    std::vector<OSM::Coordinate> terminalEntrances;
    std::vector<OSM::Coordinate> stations;
};

/** OSM airport database for optimizing geo coordinates. */
class OSMAirportDb
{
public:
    void load(const QString &path);
    OSM::Coordinate lookup(const QString &iata, float lat, float lon);

private:
    template <typename T> void loadAirport(const T &elem);
    void loadAirport(const OSM::Relation &elem, const QString &iataCode);
    void loadAirport(const OSM::Way &elem, const QString &iataCode);
    void loadTerminal(const OSM::Relation &elem);
    void loadTerminal(const OSM::Way &elem);
    void loadStation(const OSM::Node &elem);

    template <typename Iter>
    void appendPointsFromWay(QVector<QPointF> &points, const Iter &nodeEegin, const Iter &nodeEnd) const;
    OSM::Id appendNextPath(QVector<QPointF> &points, OSM::Id startNode, OSMAirportData &airport) const;
    void resolveAirportPaths(OSMAirportData &airport) const;

    OSM::DataSet m_dataset;
    std::map<QString, OSMAirportData> m_iataMap;
};

#endif // OSMAIRPORTDB_H
