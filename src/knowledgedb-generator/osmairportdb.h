/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "osm/datatypes.h"
#include "osm/element.h"

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
    void filterTerminals(OSMAirportData &airport);
    void loadStation(OSM::Element elem);
    void filterStations(OSMAirportData &airport);

    OSM::DataSet m_dataset;
    std::map<QString, OSMAirportData> m_iataMap;
};

