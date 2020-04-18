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

#include "osmairportdb.h"

#include <osm/xmlparser.h>

#include <QDebug>
#include <QFile>

template <typename T>
static bool isActiveAirport(const T &elem)
{
    // filter out airports we aren't interested in
    // not strictly needed here, but it reduces the diagnostic noise
    const auto disused = OSM::tagValue(elem, QLatin1String("disused"));
    const auto militayLanduse = OSM::tagValue(elem, QLatin1String("landuse")) == QLatin1String("military");
    if (!disused.isEmpty() || militayLanduse) {
        return false;
    }

    const auto aeroway = OSM::tagValue(elem, QLatin1String("aeroway"));
    return aeroway == QLatin1String("aerodrome");
}

template <typename T>
static bool isTerminal(const T &elem)
{
    const auto aeroway = OSM::tagValue(elem, QLatin1String("aeroway"));
    return aeroway == QLatin1String("terminal");
}

void OSMAirportDb::load(const QString &path)
{
    QFile f(path);
    if (!f.open(QFile::ReadOnly)) {
        qCritical() << "Failed to open OSM input file!" << f.errorString() << f.fileName();
        return;
    }
    OSM::XmlParser p(&m_dataset);
    p.parse(&f);

    qDebug() << "nodes:" << m_dataset.nodes.size();
    qDebug() << "ways:" << m_dataset.ways.size();
    qDebug() << "relations:" << m_dataset.relations.size();

    // find all airports
    // those can be present in multiple forms
    // as a single node: we don't care, this doesn't improve coordinate information for our use-case
    // as a single way for the outer shape
    // as a relation representing a multi-polygon outer shape
    for (const auto &rel : m_dataset.relations) {
        loadAirport(rel);
    }
    for (const auto &way : m_dataset.ways) {
        if (isActiveAirport(way)) {
            loadAirport(way);
        }
    }
    // resolve multi-path polygons
    for (auto &it : m_iataMap) {
        resolveAirportPaths(it.second);
    }

    // find all terminal buildings, and add them to their airports
    for (const auto &rel : m_dataset.relations) {
        loadTerminal(rel);
    }
    for (const auto &way : m_dataset.ways) {
        if (isTerminal(way)) {
            loadTerminal(way);
        }
    }

    // load railway stations
    for (const auto &node : m_dataset.nodes) {
        loadStation(node);
    }

    qDebug() << "airports:" << m_iataMap.size();
    qDebug() << "  with a single terminal:" << std::count_if(m_iataMap.begin(), m_iataMap.end(), [](const auto &a) { return a.second.terminalBboxes.size() == 1; } );
    qDebug() << "  with multiple terminals:" << std::count_if(m_iataMap.begin(), m_iataMap.end(), [](const auto &a) { return a.second.terminalBboxes.size() > 1; } );
    qDebug() << "  with a single entrance:" << std::count_if(m_iataMap.begin(), m_iataMap.end(), [](const auto &a) { return a.second.terminalEntrances.size() == 1; } );
    qDebug() << "  with multiple entrances:" << std::count_if(m_iataMap.begin(), m_iataMap.end(), [](const auto &a) { return a.second.terminalEntrances.size() > 1; } );
    qDebug() << "  with a single station:" <<  std::count_if(m_iataMap.begin(), m_iataMap.end(), [](const auto &a) { return a.second.stations.size() == 1; } );
    qDebug() << "  with multiple stations:" <<  std::count_if(m_iataMap.begin(), m_iataMap.end(), [](const auto &a) { return a.second.stations.size() > 1; } );
    qDebug() << "  with at least one singular feature:" <<  std::count_if(m_iataMap.begin(), m_iataMap.end(), [](const auto &a) {
        return a.second.stations.size() == 1 || a.second.terminalBboxes.size() == 1 || a.second.terminalEntrances.size() == 1;
    });
    qDebug() << "  with conflicting features:" <<  std::count_if(m_iataMap.begin(), m_iataMap.end(), [](const auto &a) {
        return a.second.stations.size() != 1 && a.second.terminalBboxes.size() != 1 && a.second.terminalEntrances.size() != 1 &&
            !(a.second.stations.empty() && a.second.terminalBboxes.empty() && a.second.terminalEntrances.empty());
    });
}

template<typename T> void OSMAirportDb::loadAirport(const T &elem)
{
    const auto iata = OSM::tagValue(elem, QLatin1String("iata"));
    if (iata.isEmpty()) {
        return;
    }

    // semicolon list split
    if (iata.contains(QLatin1Char(';'))) {
        const auto iatas = iata.split(QLatin1Char(';'), Qt::SkipEmptyParts);
        for (const auto &iata : iatas) {
            auto e = elem;
            OSM::setTagValue(e, QStringLiteral("iata"), iata);
            loadAirport(e);
        }
        return;
    }

    if (iata.size() != 3 || !std::all_of(iata.begin(), iata.end(), [](const auto c) { return c.isUpper(); })) {
        qWarning() << "IATA code format violation:" << iata << elem.url();
        return;
    }

    loadAirport(elem, iata);
}

void OSMAirportDb::loadAirport(const OSM::Relation &elem, const QString &iataCode)
{
    if (!isActiveAirport(elem)) {
        return;
    }

    const auto it = m_iataMap.find(iataCode);
    if (it != m_iataMap.end()) {
        qWarning() << "Duplicate relation for IATA code:" << iataCode << (*it).second.source << elem.url();
        return;
    }

    OSMAirportData airport;
    airport.source = elem.url();
    airport.bbox = elem.bbox;
    m_iataMap[iataCode] = std::move(airport);

    // we assume type == multipolygon here
    for (const auto &member : elem.members) {
        if (member.role != QLatin1String("outer")) {
            continue;
        }
        const auto it = std::lower_bound(m_dataset.ways.begin(), m_dataset.ways.end(), member.id);
        if (it != m_dataset.ways.end() && (*it).id == member.id) {
           loadAirport(*it, iataCode);
        }
    }
}

void OSMAirportDb::loadAirport(const OSM::Way &elem, const QString &iataCode)
{
    if (elem.nodes.empty()) {
        qWarning() << "Empty way element!" << elem.url();
        return;
    }

    const auto it = m_iataMap.find(iataCode);
    if (it != m_iataMap.end()) {
        // check if this overlaps, then it's just multiple parts of the same airport, otherwise this is a suspected IATA code duplication
        if ((*it).second.bbox.isValid() && !OSM::intersects((*it).second.bbox, elem.bbox)) {
            // TODO we probably want to exclude the entire code as invalid then
            qWarning() << "duplicate IATA code?" << (*it).first << elem.url() << (*it).second.source;
        } else {
            //qDebug() << "merging airport parts:" << iataCode << (*it).second.source << elem.url();
            (*it).second.bbox = OSM::unite((*it).second.bbox, elem.bbox);

            if (elem.isClosed()) {
                QVector<QPointF> points;
                points.reserve(elem.nodes.size());
                appendPointsFromWay(points, elem.nodes.begin(), elem.nodes.end());
                (*it).second.airportPolygon = (*it).second.airportPolygon.united(QPolygonF(points));
            } else {
                (*it).second.airportPaths.push_back(&elem);
            }
        }
        return;
    }

    //qDebug() << iata << elem.bbox << elem.id;

    OSMAirportData airport;
    airport.source = elem.url();
    airport.bbox = elem.bbox;
    if (elem.isClosed()) {
        QVector<QPointF> points;
        points.reserve(elem.nodes.size());
        appendPointsFromWay(points, elem.nodes.begin(), elem.nodes.end());
        airport.airportPolygon = QPolygonF(points);
    } else {
        airport.airportPaths.push_back(&elem);
    }
    m_iataMap[iataCode] = std::move(airport);
}

void OSMAirportDb::loadTerminal(const OSM::Relation& elem)
{
    if (!isTerminal(elem)) {
        return;
    }

    // we assume type == multipolygon here
    for (const auto &member : elem.members) {
        if (member.role != QLatin1String("outer")) {
            continue;
        }
        const auto it = std::lower_bound(m_dataset.ways.begin(), m_dataset.ways.end(), member.id);
        if (it != m_dataset.ways.end() && (*it).id == member.id) {
           loadTerminal(*it);
        }
    }
}


void OSMAirportDb::loadTerminal(const OSM::Way &elem)
{
    // find matching airport
    for (auto it = m_iataMap.begin(); it != m_iataMap.end(); ++it) {
        if (!OSM::intersects((*it).second.bbox, elem.bbox)) {
            continue;
        }
        // check against the exact airport boundary, not just the bounding box,
        // this excludes terminal buildings from adjacent sites we don't care about
        // example: the Airbus delivery buildings next to TLS
        if (!(*it).second.airportPolygon.intersects(QRectF(QPointF(elem.bbox.min.latF(), elem.bbox.min.lonF()), QPointF(elem.bbox.max.latF(), elem.bbox.max.lonF())))) {
            continue;
        }
        //qDebug() << "found terminal for airport:" << elem.url() << (*it).first << (*it).second.source;
        (*it).second.terminalBboxes.push_back(elem.bbox);

        // look for entrances to terminals
        for (const auto &nodeId : elem.nodes) {
            const auto nodeIt = std::lower_bound(m_dataset.nodes.begin(), m_dataset.nodes.end(), nodeId);
            if (nodeIt == m_dataset.nodes.end() || (*nodeIt).id != nodeId) {
                continue;
            }

            // filter out inaccessible entrances, or gates
            const auto access = OSM::tagValue(*nodeIt, QLatin1String("access"));
            const auto aeroway = OSM::tagValue(*nodeIt, QLatin1String("gate"));
            if (access == QLatin1String("private") || access == QLatin1String("no") || aeroway == QLatin1String("gate")) {
                continue;
            }

            const auto entrance = OSM::tagValue(*nodeIt, QLatin1String("entrance"));
            if (entrance == QLatin1String("yes") || entrance == QLatin1String("main")) {
                //qDebug() << "  found entrance for terminal:" << (*nodeIt).url() << entrance << access;
                (*it).second.terminalEntrances.push_back((*nodeIt).coordinate);
            }
        }
    }
}

void OSMAirportDb::loadStation(const OSM::Node &elem)
{
    const auto railway = OSM::tagValue(elem, QLatin1String("railway"));
    if (railway != QLatin1String("station")) {
        return;
    }

    // try to filter out airport-interal transport systems, those are typically airside and thus not what we want
    const auto station = OSM::tagValue(elem, QLatin1String("station"));
    if (station == QLatin1String("monorail")) {
        return;
    }

    for (auto it = m_iataMap.begin(); it != m_iataMap.end(); ++it) {
        // we need the exact path here, the bounding box can contain a lot more stuff
        // the bounding box check is just for speed
        if (!OSM::contains((*it).second.bbox, elem.coordinate)
         || !(*it).second.airportPolygon.containsPoint(QPointF(elem.coordinate.latF(), elem.coordinate.lonF()), Qt::WindingFill))
        {
            continue;
        }
        //qDebug() << "found station for airport:" << elem.url() << (*it).first << (*it).second.source;
        (*it).second.stations.push_back(elem.coordinate);
    }
}

OSM::Coordinate OSMAirportDb::lookup(const QString &iata, float lat, float lon)
{
    const auto it = m_iataMap.find(iata);
    if (it == m_iataMap.end()) {
        //qDebug() << "No airport with IATA code:" << iata;
        return {};
    }

    const OSM::Coordinate wdCoord(lat, lon);
    const auto &airport = (*it).second;
    if (!OSM::contains(airport.bbox, wdCoord)) {
        qDebug() << "Airport" << iata << "is not where we expect it to be!?" << airport.source << airport.bbox << lat << lon;
        return {};
    }

    // single entrance
    qDebug() << "Optimizing" << iata << airport.source << lat << lon << airport.bbox;
    qDebug() << "  entrances:" << airport.terminalEntrances.size() << "terminals:" << airport.terminalBboxes.size() << "stations:" << airport.stations.size();
    if (airport.terminalEntrances.size() == 1) { // ### this works for small airports, but for larger ones this is often due to missing data
        qDebug() << "  by entrance:" << airport.terminalEntrances[0];
        return airport.terminalEntrances[0];
    }

    // single terminal
    if (airport.terminalBboxes.size() == 1) {
        qDebug() << "  by terminal:" << airport.terminalBboxes[0].center();
        return airport.terminalBboxes[0].center();
    }

    // single on-premise station
    if (airport.stations.size() == 1) {
        qDebug() << "  by station:" << airport.stations[0];
        return airport.stations[0];
    }

    // multiple terminals: take the center of the sum of all bounding boxes, and TODO check the result isn't ridiculously large
    if (airport.terminalBboxes.size() > 1) {
        const auto terminalBbox = std::accumulate(airport.terminalBboxes.begin(), airport.terminalBboxes.end(), OSM::BoundingBox(), OSM::unite);
        // if the original coordinate is outside the terminal bounding box, this is highly likely an improvement,
        // otherwise we cannot be sure (see MUC, where the Wikidata coordinate is ideal).
        if (!OSM::contains(terminalBbox, wdCoord)) {
            qDebug() << "  by terminal bbox center:" << terminalBbox.center();
            return terminalBbox.center();
        }
    }

    return {};
}

template <typename Iter>
void OSMAirportDb::appendPointsFromWay(QVector<QPointF>& points, const Iter& nodeBegin, const Iter &nodeEnd) const
{
    points.reserve(points.size() + std::distance(nodeBegin, nodeEnd));
    for (auto it = nodeBegin; it != nodeEnd; ++it) {
        const auto nodeIt = std::lower_bound(m_dataset.nodes.begin(), m_dataset.nodes.end(), (*it));
        if (nodeIt == m_dataset.nodes.end() || (*nodeIt).id != (*it)) {
            continue;
        }
        points.push_back(QPointF((*nodeIt).coordinate.latF(), (*nodeIt).coordinate.lonF()));
    }
}

OSM::Id OSMAirportDb::appendNextPath(QVector<QPointF> &points, OSM::Id startNode, OSMAirportData &airport) const
{
    if (airport.airportPaths.empty()) {
        return {};
    }

    for (auto it = airport.airportPaths.begin() + 1; it != airport.airportPaths.end(); ++it) {
        assert(!(*it)->nodes.empty()); // ensured above
        //qDebug() << "    looking at:" << (*it)->nodes.front() << (*it)->url();
        if ((*it)->nodes.front() == startNode) {
            appendPointsFromWay(points, (*it)->nodes.begin(), (*it)->nodes.end());
            const auto lastNodeId = (*it)->nodes.back();
            airport.airportPaths.erase(it);
            return lastNodeId;
        }
        // path segments can also be backwards
        if ((*it)->nodes.back() == startNode) {
            appendPointsFromWay(points, (*it)->nodes.rbegin(), (*it)->nodes.rend());
            const auto lastNodeId = (*it)->nodes.front();
            airport.airportPaths.erase(it);
            return lastNodeId;
        }
    }

    return {};
}

void OSMAirportDb::resolveAirportPaths(OSMAirportData &airport) const
{
    //qDebug() << "resolving polygon for" << airport.source << airport.bbox << airport.airportPaths.size();
    for (auto it = airport.airportPaths.begin(); it != airport.airportPaths.end();) {
        assert(!(*it)->nodes.empty()); // ensured above
        if (airport.airportPaths.size() == 1) {
            qWarning() << "  open airport polgyon:" << airport.source << (*it)->url();
            return;
        }

        QVector<QPointF> points;
        appendPointsFromWay(points, (*it)->nodes.begin(), (*it)->nodes.end());
        const auto startNode = (*it)->nodes.front();
        auto lastNode = (*it)->nodes.back();
        //qDebug() << "  starting:" << startNode << lastNode << (*it)->url();

        do {
            lastNode = appendNextPath(points, lastNode, airport);
            //qDebug() << "  next:" << lastNode;
        } while (lastNode && lastNode != startNode);

        if (lastNode != startNode) {
            qWarning() << "  open airport polygon:" << airport.source << (*it)->url();
        } else {
            airport.airportPolygon = airport.airportPolygon.united(QPolygonF(points));
        }

        it = airport.airportPaths.erase(it);
    }
    //qDebug() << "  polygon:" << airport.airportPolygon.size() << airport.airportPolygon.boundingRect();
}
