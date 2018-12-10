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

#include "trainstationdbgenerator.h"
#include "codegen.h"
#include "util.h"
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
    if (!fetchIBNR() || !fetchGaresConnexions() || !fetchCountryInformation()) {
        return false;
    }

    // timezone lookup and filtering
    processStations();

    // code generation
    CodeGen::writeLicenseHeader(out);
    out->write(R"(
#include "knowledgedb.h"
#include "timezonedb.h"
#include "trainstationdb.h"
#include "timezonedb_data_p.h"

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

    printSummary();
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

        const auto id = stationObj.value(QLatin1String("ibnr")).toObject().value(QLatin1String("value")).toString().toUInt();
        if (id < 1000000 || id > 9999999) {
            ++m_idFormatViolations;
            qWarning() << "IBNR format violation" << id << uri;
            continue;
        }

        const auto it = m_ibnrMap.find(id);
        if (it != m_ibnrMap.end() && (*it).second != uri) {
            ++m_idConflicts;
            qWarning() << "Conflict on IBNR" << id << uri << m_ibnrMap[id];
        } else {
            m_ibnrMap[id] = uri;
        }
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

        const auto id = stationObj.value(QLatin1String("gareConnexionId")).toObject().value(QLatin1String("value")).toString().toUpper();
        if (id.size() != 5 || !Util::containsOnlyLetters(id)) {
            ++m_idFormatViolations;
            qWarning() << "Gares & Connexions ID format violation" << id << uri;
            continue;
        }

        const auto it = m_garesConnexionsIdMap.find(id);
        if (it != m_garesConnexionsIdMap.end() && (*it).second != uri) {
            ++m_idConflicts;
            qWarning() << "Conflict on Gares & Connexions ID" << id << uri << m_garesConnexionsIdMap[id];
        } else {
            m_garesConnexionsIdMap[id] = uri;
        }
    }

    return true;
}

bool TrainStationDbGenerator::fetchCountryInformation()
{
    const auto stationArray = WikiData::query(R"(
        SELECT DISTINCT ?station ?isoCode WHERE {
            ?station (wdt:P31/wdt:P279*) wd:Q55488.
            ?station wdt:P17 ?country.
            ?country wdt:P297 ?isoCode.
        } ORDER BY (?station))", "wikidata_trainstation_country.json");
    if (stationArray.isEmpty()) {
        qWarning() << "Empty query result!";
        return false;
    }

    for (const auto &stationData : stationArray) {
        const auto uri = insertOrMerge(stationData.toObject(), true);
    }

    return true;
}

QUrl TrainStationDbGenerator::insertOrMerge(const QJsonObject &obj, bool mergeOnly)
{
    if (obj.isEmpty()) {
        return {};
    }

    Station s;
    s.uri = QUrl(obj.value(QLatin1String("station")).toObject().value(QLatin1String("value")).toString());
    s.name = obj.value(QLatin1String("stationLabel")).toObject().value(QLatin1String("value")).toString();
    s.coord = WikiData::parseCoordinate(obj.value(QLatin1String("coord")).toObject().value(QLatin1String("value")).toString());
    s.isoCode = obj.value(QLatin1String("isoCode")).toObject().value(QLatin1String("value")).toString();

    const auto it = std::lower_bound(m_stations.begin(), m_stations.end(), s);
    if (it != m_stations.end() && (*it).uri == s.uri) {
        if ((*it).name.isEmpty()) {
            (*it).name = s.name;
        }
        // check for coordinate conflicts
        if (s.coord.isValid() && (*it).coord.isValid()) {
            if (std::abs(s.coord.latitude - (*it).coord.latitude) > 0.2f || std::abs(s.coord.longitude - (*it).coord.longitude) > 0.2f) {
                ++m_coordinateConflicts;
                qWarning() << s.uri << "has multiple conflicting coordinates";
            }
            // pick always the same independent of the input order, so stabilize generated output
            (*it).coord.latitude = std::min((*it).coord.latitude, s.coord.latitude);
            (*it).coord.longitude = std::min((*it).coord.longitude, s.coord.longitude);
        }
        if ((*it).isoCode != s.isoCode && !s.isoCode.isEmpty()) {
            if (!(*it).isoCode.isEmpty()) {
                ++m_countryConflicts;
                qWarning() << s.uri << (*it).name << "has multiple country codes";
            } else {
                (*it).isoCode = s.isoCode;
            }
        }

        return s.uri;
    }

    if (!mergeOnly) {
        m_stations.insert(it, s);
    }
    return s.uri;
}

void TrainStationDbGenerator::processStations()
{
    for (auto it = m_stations.begin(); it != m_stations.end();) {
        if (!(*it).coord.isValid()) {
            qDebug() << "Station has no geo coordinates:" << (*it).name << (*it).uri;
        }

        (*it).tz = m_tzDb.timezoneForLocation((*it).isoCode, (*it).coord);
        if ((*it).tz.isEmpty() && ((*it).coord.isValid() || !(*it).isoCode.isEmpty())) {
            ++m_timezoneLookupFailure;
            qWarning() << "Timezone lookup failure:" << (*it).name << (*it).uri;
        }

        if (!(*it).coord.isValid() && (*it).tz.isEmpty() && (*it).isoCode.isEmpty()) { // no useful information
            it = m_stations.erase(it);
        } else {
            ++it;
        }
    }
}

void TrainStationDbGenerator::writeStationData(QIODevice *out)
{
    out->write("static const TrainStation trainstation_table[] = {\n");
    for (const auto &station : m_stations) {
        out->write("    {");
        CodeGen::writeCoordinate(out, station.coord);
        out->write(", ");
        CodeGen::writeTimezone(out, station.tz);
        out->write(", ");
        CodeGen::writeCountryIsoCode(out, station.isoCode);
        out->write("}, // ");
        out->write(station.name.toUtf8());
        out->write("\n");
    }
    out->write("};\n\n");
}

void TrainStationDbGenerator::writeIBNRMap(QIODevice *out)
{
    out->write("static const IBNR ibnr_table[] = {\n");
    for (const auto &it : m_ibnrMap) {
        const auto station = std::lower_bound(m_stations.begin(), m_stations.end(), it.second);
        if (station == m_stations.end() || (*station).uri != it.second) {
            continue;
        }
        out->write("    IBNR{");
        out->write(QByteArray::number(it.first));
        out->write("}, // ");
        out->write((*station).name.toUtf8());
        out->write("\n");
    }
    out->write("};\n\n");

    out->write("static const TrainStationIndex ibnr_index[] = {\n");
    for (const auto &it : m_ibnrMap) {
        const auto station = std::lower_bound(m_stations.begin(), m_stations.end(), it.second);
        if (station == m_stations.end() || (*station).uri != it.second) {
            continue;
        }
        out->write("    ");
        out->write(QByteArray::number((int)std::distance(m_stations.begin(), station)));
        out->write(", // ");
        out->write(QByteArray::number(it.first));
        out->write(" -> ");
        out->write((*station).name.toUtf8());
        out->write("\n");
    }
    out->write("};\n\n");
}

void TrainStationDbGenerator::writeGareConnexionMap(QIODevice *out)
{
    out->write("static const GaresConnexionsId garesConnexionsId_table[] = {\n");
    for (const auto &it : m_garesConnexionsIdMap) {
        const auto station = std::lower_bound(m_stations.begin(), m_stations.end(), it.second);
        if (station == m_stations.end() || (*station).uri != it.second) {
            continue;
        }
        out->write("    GaresConnexionsId{\"");
        out->write(it.first.toUtf8());
        out->write("\"}, // ");
        out->write((*station).name.toUtf8());
        out->write("\n");
    }
    out->write("};\n\n");

    out->write("static const TrainStationIndex garesConnexionsId_index[] = {\n");
    for (const auto &it : m_garesConnexionsIdMap) {
        const auto station = std::lower_bound(m_stations.begin(), m_stations.end(), it.second);
        if (station == m_stations.end() || (*station).uri != it.second) {
            continue;
        }
        out->write("    ");
        out->write(QByteArray::number((int)std::distance(m_stations.begin(), station)));
        out->write(", // ");
        out->write(it.first.toUtf8());
        out->write(" -> ");
        out->write((*station).name.toUtf8());
        out->write("\n");
    }
    out->write("};\n\n");
}

void TrainStationDbGenerator::printSummary()
{
    qDebug() << "Generated database containing" << m_stations.size() << "train stations";
    qDebug() << "IBNR index:" << m_ibnrMap.size() << "elements";
    qDebug() << "Gares & Connexions ID index:" << m_garesConnexionsIdMap.size() << "elements";
    qDebug() << "Identifier collisions:" << m_idConflicts;
    qDebug() << "Identifier format violations:" << m_idFormatViolations;
    qDebug() << "Coordinate conflicts:" << m_coordinateConflicts;
    qDebug() << "Country ISO code conflicts: " << m_countryConflicts;
    qDebug() << "Failed timezone lookups:" << m_timezoneLookupFailure;
}
