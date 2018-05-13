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

#include "trainstationdbgenerator.h"
#include "codegen.h"
#include "wikidata.h"

#include <QByteArray>
#include <QDebug>
#include <QIODevice>
#include <QJsonArray>
#include <QJsonObject>

using namespace KItinerary::Generator;

namespace KItinerary {
namespace Generator {

static bool operator<(const TrainStationDbGenerator::Station &lhs, const TrainStationDbGenerator::Station &rhs)
{
    return lhs.uri < rhs.uri;
}
static bool operator<(const TrainStationDbGenerator::Station &lhs, const QUrl &rhs)
{
    return lhs.uri < rhs;
}

}
}

bool TrainStationDbGenerator::generate(QIODevice *out)
{
    // retrieve content from Wikidata
    if (!fetchIBNR() || !fetchGaresConnexions()) {
        return false;
    }

    // timezone lookup
    for (auto it = m_stations.begin(); it != m_stations.end(); ++it) {
        // TODO warn about lookup failures
        (*it).tz = m_tzDb.timezoneForCoordinate((*it).coord);
    }

    // code generation
    CodeGen::writeLicenseHeader(out);
    out->write(R"(
#include "knowledgedb.h"
#include "timezonedb.h"
#include "trainstationdb.h"

namespace KItinerary {
namespace KnowledgeDb {
)");
    writeStationData(out);
    writeIBNRMap(out);
    writeGareConnexionMap(out);
    out->write(R"(
}
}
)");

    return true;
}

bool TrainStationDbGenerator::fetchIBNR()
{
    const auto stationArray = WikiData::query(R"(
        SELECT DISTINCT ?station ?stationLabel ?ibnr ?coord WHERE {
            ?station (wdt:P31/wdt:P279*) wd:Q55488.
            ?station wdt:P954 ?ibnr.
            OPTIONAL { ?station wdt:P625 ?coord. }
            SERVICE wikibase:label { bd:serviceParam wikibase:language "en". }
        } ORDER BY (?station))", "wikidata_trainstation_ibnr.json");
    if (stationArray.isEmpty()) {
        qWarning() << "Empty query result!";
        return false;
    }

    for (const auto &stationData : stationArray) {
        const auto stationObj = stationData.toObject();
        const auto uri = insertOrMerge(stationObj);
        // TODO handle IBNR conflicts or format issues
        m_ibnrMap[stationObj.value(QLatin1String("ibnr")).toObject().value(QLatin1String("value")).toString().toUInt()] = uri;
    }

    return true;
}

bool TrainStationDbGenerator::fetchGaresConnexions()
{
    const auto stationArray = WikiData::query(R"(
        SELECT DISTINCT ?station ?stationLabel ?gareConnexionId ?coord WHERE {
            ?station (wdt:P31/wdt:P279*) wd:Q55488.
            ?station wdt:P3104 ?gareConnexionId.
            OPTIONAL { ?station wdt:P625 ?coord. }
            SERVICE wikibase:label { bd:serviceParam wikibase:language "en". }
        } ORDER BY (?station))", "wikidata_trainstation_gare_connexion.json");
    if (stationArray.isEmpty()) {
        qWarning() << "Empty query result!";
        return false;
    }

    for (const auto &stationData : stationArray) {
        const auto stationObj = stationData.toObject();
        const auto uri = insertOrMerge(stationObj);
        // TODO handle id conflicts or format issues
        m_garesConnexionsIdMap[stationObj.value(QLatin1String("gareConnexionId")).toObject().value(QLatin1String("value")).toString().toUpper()] = uri;
    }

    return true;
}

QUrl TrainStationDbGenerator::insertOrMerge(const QJsonObject &obj)
{
    if (obj.isEmpty()) {
        return {};
    }

    Station s;
    s.uri = QUrl(obj.value(QLatin1String("station")).toObject().value(QLatin1String("value")).toString());
    s.name = obj.value(QLatin1String("stationLabel")).toObject().value(QLatin1String("value")).toString();
    s.coord = WikiData::parseCoordinate(obj.value(QLatin1String("coord")).toObject().value(QLatin1String("value")).toString());

    const auto it = std::lower_bound(m_stations.begin(), m_stations.end(), s);
    if (it != m_stations.end() && (*it).uri == s.uri) {
        // TODO merge: check for coordinate conflicts
        return s.uri;
    }

    m_stations.insert(it, s);
    return s.uri;
}

void TrainStationDbGenerator::writeStationData(QIODevice *out)
{
    out->write("static const TrainStation trainstation_table[] = {\n");
    for (const auto &station : m_stations) {
        out->write("    {");
        CodeGen::writeCoordinate(out, station.coord);
        out->write(", ");
        CodeGen::writeTimezone(out, &m_tzDb, station.tz);
        out->write("}, // ");
        out->write(station.name.toUtf8());
        out->write("\n");
    }
    out->write("};\n\n");
}

void TrainStationDbGenerator::writeIBNRMap(QIODevice *out)
{
    out->write("static const IBNR ibnr_table[] = {\n");
    for (auto it = m_ibnrMap.begin(); it != m_ibnrMap.end(); ++it) {
        const auto station = std::lower_bound(m_stations.begin(), m_stations.end(), it->second);
        out->write("    IBNR{");
        out->write(QByteArray::number(it->first));
        out->write("}, // ");
        out->write((*station).name.toUtf8());
        out->write("\n");
    }
    out->write("};\n\n");

    out->write("static const TrainStationIndex ibnr_index[] = {\n");
    for (auto it = m_ibnrMap.begin(); it != m_ibnrMap.end(); ++it) {
        const auto station = std::lower_bound(m_stations.begin(), m_stations.end(), it->second);
        out->write("    ");
        out->write(QByteArray::number((int)std::distance(m_stations.begin(), station)));
        out->write(", // ");
        out->write(QByteArray::number(it->first));
        out->write(" -> ");
        out->write((*station).name.toUtf8());
        out->write("\n");
    }
    out->write("};\n\n");
}

void TrainStationDbGenerator::writeGareConnexionMap(QIODevice *out)
{
    out->write("static const GaresConnexionsId garesConnexionsId_table[] = {\n");
    for (auto it = m_garesConnexionsIdMap.begin(); it != m_garesConnexionsIdMap.end(); ++it) {
        const auto station = std::lower_bound(m_stations.begin(), m_stations.end(), it->second);
        out->write("    GaresConnexionsId{\"");
        out->write(it->first.toUtf8());
        out->write("\"}, // ");
        out->write((*station).name.toUtf8());
        out->write("\n");
    }
    out->write("};\n\n");

    out->write("static const TrainStationIndex garesConnexionsId_index[] = {\n");
    for (auto it = m_garesConnexionsIdMap.begin(); it != m_garesConnexionsIdMap.end(); ++it) {
        const auto station = std::lower_bound(m_stations.begin(), m_stations.end(), it->second);
        out->write("    ");
        out->write(QByteArray::number((int)std::distance(m_stations.begin(), station)));
        out->write(", // ");
        out->write(it->first.toUtf8());
        out->write(" -> ");
        out->write((*station).name.toUtf8());
        out->write("\n");
    }
    out->write("};\n\n");
}
