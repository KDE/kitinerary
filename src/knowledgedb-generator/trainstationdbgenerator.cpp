/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "trainstationdbgenerator.h"
#include "codegen.h"
#include "util.h"
#include "wikidata.h"

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
    if (!fetch("P954", "ibnr", m_ibnrMap)
     || !fetch("P722", "uic", m_uicMap)
     || !fetch("P8181", "sncf", m_sncfIdMap)
     || !fetch("P8448", "benerail", m_benerailIdMap)
     || !fetchIndianRailwaysStationCode()
     || !fetchFinishStationCodes()
    ) {
        return false;
    }
    if (!fetchCountryInformation()) {
         return false;
     }

    // filtering out stations without useful information
    processStations();

    // code generation
    CodeGen::writeLicenseHeader(out);
    out->write(R"(
#include "knowledgedb.h"
#include "trainstationdb.h"

namespace KItinerary {
namespace KnowledgeDb {
)");
    writeStationData(out);
    writeIdMap(out, m_ibnrMap, "ibnr", "IBNR");
    writeIdMap(out, m_uicMap, "uic", "UICStation");
    writeIdMap(out, m_sncfIdMap, "sncfStationId", "SncfStationId");
    writeIdMap(out, m_benerailIdMap, "benerail", "BenerailStationId");
    writeIndianRailwaysMap(out);
    writeVRMap(out);
    out->write(R"(
}
}
)");

    printSummary();
    return true;
}

template<typename Id>
bool TrainStationDbGenerator::fetch(const char *prop, const char *name, std::map<Id, QUrl> &idMap)
{
    const auto stationArray = WikiData::query(QLatin1String(R"(
        SELECT DISTINCT ?station ?stationLabel ?id ?coord ?replacedBy WHERE {
            ?station (wdt:P31/wdt:P279*) wd:Q55488.
            ?station wdt:)") + QString::fromUtf8(prop) + QLatin1String(R"( ?id.
            OPTIONAL { ?station wdt:P625 ?coord. }
            OPTIONAL { ?station wdt:P1366 ?replacedBy. }
            SERVICE wikibase:label { bd:serviceParam wikibase:language "en". }
        } ORDER BY (?station))"), QLatin1String("wikidata_trainstation_") + QString::fromUtf8(name) + QLatin1String(".json"));
    if (stationArray.isEmpty()) {
        qWarning() << "Empty query result!";
        return false;
    }

    for (const auto &stationData : stationArray) {
        const auto stationObj = stationData.toObject();
        if (stationObj.contains(QLatin1String("replacedBy"))) {
            continue;
        }

        const auto uri = insertOrMerge(stationObj);
        const auto idStr = stationObj.value(QLatin1String("id")).toObject().value(QLatin1String("value")).toString();
        const auto id = Id(idStr);
        if (!id.isValid()) {
            ++m_idFormatViolations;
            qWarning() << name << "format violation" << idStr << uri;
            continue;
        }

        const auto it = idMap.find(id);
        if (it != idMap.end() && (*it).second != uri) {
            ++m_idConflicts;
            qWarning() << "Conflict on" << name << idStr << uri << idMap[id];
        } else {
            idMap[id] = uri;
        }
    }

    return true;
}

bool TrainStationDbGenerator::fetchIndianRailwaysStationCode()
{
    const auto stationArray = WikiData::query(R"(
        SELECT DISTINCT ?station ?stationLabel ?irId ?coord WHERE {
            ?station (wdt:P31/wdt:P279*) wd:Q55488.
            ?station wdt:P5696 ?irId.
            OPTIONAL { ?station wdt:P625 ?coord. }
            SERVICE wikibase:label { bd:serviceParam wikibase:language "en". }
        } ORDER BY (?station))", "wikidata_trainstation_indian_railways.json");
    if (stationArray.isEmpty()) {
        qWarning() << "Empty query result!";
        return false;
    }

    for (const auto &stationData : stationArray) {
        const auto stationObj = stationData.toObject();
        const auto uri = insertOrMerge(stationObj);

        const auto id = stationObj.value(QLatin1String("irId")).toObject().value(QLatin1String("value")).toString().toUpper();
        const auto it = m_indianRailwaysMap.find(id);
        if (it != m_indianRailwaysMap.end() && (*it).second != uri) {
            ++m_idConflicts;
            qWarning() << "Conflict on Indian Railwaiys station code" << id << uri << m_indianRailwaysMap[id];
        } else {
            m_indianRailwaysMap[id] = uri;
        }
    }

    return true;
}

bool TrainStationDbGenerator::fetchFinishStationCodes()
{
    const auto stationArray = WikiData::query(R"(
        SELECT DISTINCT ?station ?stationLabel ?code ?coord ?ref WHERE {
            ?station (wdt:P31/wdt:P279*) wd:Q55488.
            ?station p:P296 ?codeStmt.
            ?codeStmt ps:P296 ?code.
            ?codeStmt prov:wasDerivedFrom ?refnode.
            ?refnode pr:P854 ?ref.
            OPTIONAL { ?station wdt:P625 ?coord. }
            SERVICE wikibase:label { bd:serviceParam wikibase:language "en". }
        } ORDER BY (?station))", "wikidata_trainstation_vrfi.json");
    if (stationArray.isEmpty()) {
        qWarning() << "Empty query result!";
        return false;
    }

    for (const auto &stationData : stationArray) {
        const auto stationObj = stationData.toObject();
        const auto ref = stationObj.value(QLatin1String("ref")).toObject().value(QLatin1String("value")).toString();
        if (!ref.contains(QLatin1String("rata.digitraffic.fi"), Qt::CaseInsensitive)) {
            continue;
        }
        const auto uri = insertOrMerge(stationObj);

        // TODO this filters 'Ä' and 'Ö' too, which seem to occur in a few cases?
        const auto idStr = stationObj.value(QLatin1String("code")).toObject().value(QLatin1String("value")).toString().toUpper();
        const auto id = KnowledgeDb::VRStationCode(idStr);
        if (!id.isValid()) {
            ++m_idFormatViolations;
            qWarning() << "VR (Finland) station id format violation" << idStr << uri;
            continue;
        }

        const auto it = m_vrfiMap.find(id);
        if (it != m_vrfiMap.end() && (*it).second != uri) {
            ++m_idConflicts;
            qWarning() << "Conflict on VR (Finland) station code" << idStr << uri << m_vrfiMap[id];
        } else {
            m_vrfiMap[id] = uri;
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
        Q_UNUSED(uri)
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

        if (!(*it).coord.isValid() && (*it).isoCode.isEmpty()) { // no useful information
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
        CodeGen::writeCountryIsoCode(out, station.isoCode);
        out->write("}, // ");
        out->write(station.name.toUtf8());
        out->write("\n");
    }
    out->write("};\n\n");
}

template<typename Id>
void TrainStationDbGenerator::writeIdMap(QIODevice *out, const std::map<Id, QUrl> &idMap, const char *tabName, const char *typeName) const
{
    out->write("static constexpr const TrainStationIdIndex<");
    out->write(typeName);
    out->write("> ");
    out->write(tabName);
    out->write("_table[] = {\n");
    for (const auto &it : idMap) {
        const auto station = std::lower_bound(m_stations.begin(), m_stations.end(), it.second);
        if (station == m_stations.end() || (*station).uri != it.second) {
            continue;
        }
        out->write("    { ");
        out->write(typeName);
        out->write("{");
        out->write(encodeId(it.first));
        out->write("}, TrainStationIndex{");
        out->write(QByteArray::number((int)std::distance(m_stations.begin(), station)));
        out->write("} }, // ");
        out->write((*station).name.toUtf8());
        out->write("\n");
    }
    out->write("};\n\n");
}

void TrainStationDbGenerator::writeIndianRailwaysMap(QIODevice *out)
{
    // variable length identifiers, so we need a string table
    std::vector<uint16_t> offsets;
    offsets.reserve(m_indianRailwaysMap.size());
    uint16_t offset = 0;

    out->write("static constexpr const char indianRailwaysSationCode_stringtable[] =\n");
    for (const auto &it : m_indianRailwaysMap) {
        const auto station = std::lower_bound(m_stations.begin(), m_stations.end(), it.second);
        if (station == m_stations.end() || (*station).uri != it.second) {
            continue;
        }

        offsets.push_back(offset);
        out->write("    \"");
        out->write(it.first.toUtf8());
        out->write("\\0\" // ");
        out->write((*station).name.toUtf8());
        out->write("\n");

        offset += it.first.toUtf8().size() + 1; // +1 for the terminating null byte
    }
    out->write(";\n\n");

    out->write(
R"(static constexpr const struct {
    uint16_t offset;
    TrainStationIndex stationIndex;
} indianRailwaysSationCode_index[] = {
)");
    int offsetIdx = 0;
    for (const auto &it : m_indianRailwaysMap) {
        const auto station = std::lower_bound(m_stations.begin(), m_stations.end(), it.second);
        if (station == m_stations.end() || (*station).uri != it.second) {
            continue;
        }

        out->write("    { ");
        out->write(QByteArray::number(offsets[offsetIdx++]));
        out->write(", TrainStationIndex{");
        out->write(QByteArray::number((int)std::distance(m_stations.begin(), station)));
        out->write("} }, // ");
        out->write(it.first.toUtf8());
        out->write("\n");
    }
    out->write("};\n\n");
}

void TrainStationDbGenerator::writeVRMap(QIODevice *out)
{
    out->write("static constexpr const TrainStationIdIndex<VRStationCode> vrfiConnexionsId_table[] = {\n");
    for (const auto &it : m_vrfiMap) {
        const auto station = std::lower_bound(m_stations.begin(), m_stations.end(), it.second);
        if (station == m_stations.end() || (*station).uri != it.second) {
            continue;
        }
        out->write("    { VRStationCode{\"");
        out->write(it.first.toString().toUtf8());
        for (int i = 0; i < 4 - it.first.toString().toUtf8().size(); ++i) {
            out->write("\\0");
        }
        out->write("\"}, TrainStationIndex{");
        out->write(QByteArray::number((int)std::distance(m_stations.begin(), station)));
        out->write("} }, // ");
        out->write((*station).name.toUtf8());
        out->write("\n");
    }
    out->write("};\n\n");
}

void TrainStationDbGenerator::printSummary()
{
    qDebug() << "Generated database containing" << m_stations.size() << "train stations";
    qDebug() << "IBNR index:" << m_ibnrMap.size() << "elements";
    qDebug() << "UIC index:" << m_uicMap.size() << "elements";
    qDebug() << "SNCF station code index:" << m_sncfIdMap.size() << "elements";
    qDebug() << "Benerail station code index:" << m_benerailIdMap.size() << "elements";
    qDebug() << "Indian Railwaiys station code index:" << m_indianRailwaysMap.size() << "elements";
    qDebug() << "VR (Finland) station code index:" << m_vrfiMap.size() << "elements";
    qDebug() << "Identifier collisions:" << m_idConflicts;
    qDebug() << "Identifier format violations:" << m_idFormatViolations;
    qDebug() << "Coordinate conflicts:" << m_coordinateConflicts;
    qDebug() << "Country ISO code conflicts: " << m_countryConflicts;
}
