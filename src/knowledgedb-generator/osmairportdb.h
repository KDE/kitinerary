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
#include <osm/element.h>

#include <QPolygonF>

#include <map>
#include <vector>

struct OSMAirportData
{
    QString source; // OSM URL, for diagnostics only
    OSM::BoundingBox bbox;

    QPolygonF airportPolygon;

    std::vector<OSM::Element> terminals;
    std::vector<OSM::Coordinate> terminalEntrances;
    std::vector<OSM::Element> stations;
};

/** OSM airport database for optimizing geo coordinates. */
class OSMAirportDb
{
public:
    void load(const QString &path);
    OSM::Coordinate lookup(const QString &iata, float lat, float lon);

private:
    void loadAirport(OSM::Element elem);
    void loadAirport(OSM::Element elem, const QString &iataCode);
    void loadTerminal(OSM::Element elem);
    void loadStation(OSM::Element elem);
    void filterStations(OSMAirportData &airport);

    OSM::DataSet m_dataset;
    std::map<QString, OSMAirportData> m_iataMap;
};

#endif // OSMAIRPORTDB_H
