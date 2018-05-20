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

#include "airportdbgenerator.h"
#include "codegen.h"
#include "timezones.h"
#include "wikidata.h"

#include <airportdb_p.h>

#include <QDebug>
#include <QIODevice>
#include <QJsonArray>
#include <QJsonObject>
#include <QRegularExpression>

using namespace KItinerary;
using namespace KItinerary::AirportDb;
using namespace KItinerary::Generator;

static bool soundsMilitaryish(const QString &s)
{
    return s.contains(QLatin1String("Airbase"), Qt::CaseInsensitive)
           || s.contains(QLatin1String("Air Base"), Qt::CaseInsensitive)
           || s.contains(QLatin1String("Air Force"), Qt::CaseInsensitive)
           || s.contains(QLatin1String("Air National Guard Base"), Qt::CaseInsensitive)
           || s.contains(QLatin1String("Air Reserve Base"), Qt::CaseInsensitive)
           || s.contains(QLatin1String("Air Station"), Qt::CaseInsensitive)
           || s.contains(QLatin1String("Army Airfield"), Qt::CaseInsensitive)
           || s.contains(QLatin1String("Army Airport"), Qt::CaseInsensitive)
           || s.contains(QLatin1String("Army Heliport"), Qt::CaseInsensitive)
           || s.contains(QLatin1String("Canadian Forces Base"), Qt::CaseInsensitive)
           || s.contains(QLatin1String("Joint Base"), Qt::CaseInsensitive)
           || s.contains(QLatin1String("Marine Corps"), Qt::CaseInsensitive)
           || s.contains(QLatin1String("Military "))
           || s.contains(QLatin1String("Naval "))
           || s.contains(QLatin1String("RAF "))
           || s.contains(QLatin1String("RAAF "))
           || s.contains(QLatin1String("RNAS "))
           || s.contains(QLatin1String("CFB "))
           || s.contains(QLatin1String("PAF "))
           || s.contains(QLatin1String("NAF "))
    ;
}

static void stripAirportAllLanguages(QStringList &s)
{
    // only languages used in the English (sic!) wikidata labels and description matter here
    s.removeAll(QLatin1String("aeroport"));
    s.removeAll(QLatin1String("aeroporto"));
    s.removeAll(QLatin1String("aeropuerto"));
    s.removeAll(QLatin1String("air"));
    s.removeAll(QLatin1String("airfield"));
    s.removeAll(QLatin1String("airpark"));
    s.removeAll(QLatin1String("airport"));
    s.removeAll(QLatin1String("airstrip"));
    s.removeAll(QLatin1String("flughafen"));
    s.removeAll(QLatin1String("lufthavn"));
    s.removeAll(QLatin1String("terminal"));
}

void AirportDbGenerator::merge(Airport &lhs, const Airport &rhs)
{
    if (lhs.iataCode != rhs.iataCode) {
        qWarning() << "Multiple IATA codes on" << lhs.uri; // this can actually be valid, see BSL/MLH/EAP
    }
    // we don't really care about multiple ICAO codes
//     if (lhs.icaoCode != rhs.icaoCode)
//         qWarning() << "Multiple ICAO codes on" << lhs.uri;
    QString extraLabel;
    if (lhs.label != rhs.label) {
        extraLabel += QLatin1Char(' ') + rhs.label;
    }
    if (lhs.alias != rhs.alias) {
        extraLabel += QLatin1Char(' ') + rhs.alias;
    }
    lhs.alias += extraLabel;
    if (!lhs.coord.isValid()) {
        lhs.coord = rhs.coord;
    } else if (rhs.coord.isValid()) {
        if (std::abs(lhs.coord.latitude - rhs.coord.latitude) > 0.2f || std::abs(lhs.coord.longitude - rhs.coord.longitude) > 0.2f) {
            ++m_coordinateConflicts;
            qDebug() << lhs.uri << "has multiple conflicting coordinates";
        }
        // pick always the same independent of the input order, so stabilize generated output
        lhs.coord.latitude = std::min(lhs.coord.latitude, rhs.coord.latitude);
        lhs.coord.longitude = std::min(lhs.coord.longitude, rhs.coord.longitude);
    }
}

void AirportDbGenerator::generate(QIODevice* out)
{
    // step 1 query wikidata for all airports
    // sorted by URI to stabilize the result in case of conflicts
    const auto airportArray = WikiData::query(R"(
        SELECT DISTINCT ?airport ?airportLabel ?airportAltLabel ?iataCode ?icaoCode ?coord ?endDate ?demolished ?iataEndDate WHERE {
            ?airport (wdt:P31/wdt:P279*) wd:Q1248784.
            ?airport p:P238 ?iataStmt.
            ?iataStmt ps:P238 ?iataCode.
            OPTIONAL { ?airport wdt:P239 ?icaoCode. }
            OPTIONAL { ?airport wdt:P625 ?coord. }
            OPTIONAL { ?airport wdt:P582 ?endDate. }
            OPTIONAL { ?airport wdt:P576 ?demolished. }
            OPTIONAL { ?iataStmt pq:P582 ?iataEndDate. }
            SERVICE wikibase:label { bd:serviceParam wikibase:language "en". }
        } ORDER BY (?airport))", "wikidata_airports.json");
    if (airportArray.isEmpty()) {
        qWarning() << "No results in SPARQL query found.";
        exit(1);
    }

    for (const auto &data: airportArray) {
        const auto obj = data.toObject();
        if (obj.isEmpty()) {
            continue;
        }

        if (obj.contains(QLatin1String("endDate")) || obj.contains(QLatin1String("demolished")) || obj.contains(QLatin1String("iataEndDate"))) {
            // skip closed airports or those with expired IATA codes
            continue;
        }

        Airport a;
        a.uri = QUrl(obj.value(QLatin1String("airport")).toObject().value(QLatin1String("value")).toString());
        a.iataCode = obj.value(QLatin1String("iataCode")).toObject().value(QLatin1String("value")).toString();
        if (a.iataCode.size() != 3 || !a.iataCode.at(0).isUpper() || !a.iataCode.at(1).isUpper() || !a.iataCode.at(2).isUpper()) {
            // invalid IATA code
            continue;
        }
        a.icaoCode = obj.value(QLatin1String("icaoCode")).toObject().value(QLatin1String("value")).toString();
        a.label = obj.value(QLatin1String("airportLabel")).toObject().value(QLatin1String("value")).toString();
        a.alias = obj.value(QLatin1String("airportAltLabel")).toObject().value(QLatin1String("value")).toString();
        // primitive military airport filter, turns out to be more reliable than querying for the military airport types
        if (soundsMilitaryish(a.label) || soundsMilitaryish(a.alias)) {
            continue;
        }
        a.coord = WikiData::parseCoordinate(obj.value(QLatin1String("coord")).toObject().value(QLatin1String("value")).toString());

        // merge multiple records for the same airport
        auto it = m_airportMap.find(a.uri);
        if (it != m_airportMap.end()) {
            merge(*it, a);
            // continue nevertheless, to deal with multiple IATA codes per airport (e.g. BSL/MLH/EAP)
        } else {
            m_airportMap.insert(a.uri, a);
        }

        // TODO deal with IATA code duplications
        if (m_iataMap.contains(a.iataCode) && m_iataMap.value(a.iataCode) != a.uri) {
            ++m_iataCollisions;
            qDebug() << "duplicate iata code:" << a.iataCode << a.label << a.uri << m_airportMap.value(m_iataMap.value(a.iataCode)).label << m_airportMap.value(m_iataMap.value(a.iataCode)).uri;
        }
        m_iataMap.insert(a.iataCode, a.uri);
    }

    // step 2 augment the data with timezones
    Timezones tzDb;
    for (auto it = m_airportMap.begin(); it != m_airportMap.end(); ++it) {
        if (!(*it).coord.isValid()) {
            continue;
        }
        (*it).tz = tzDb.timezoneForCoordinate((*it).coord);
        if ((*it).tz.isEmpty()) {
            qDebug() << "Failed to find timezone for" << (*it).iataCode << (*it).label << (*it).coord.latitude << (*it).coord.longitude << (*it).uri;
            ++m_timezoneLoopupFails;
            continue;
        }
    }

    // step 3 index the names for reverse lookup
    QMap<QString, QVector<QString> > labelMap;
    for (auto it = m_airportMap.begin(); it != m_airportMap.end(); ++it) {
        auto l = QString(it.value().label + QLatin1Char(' ') + it.value().alias)
                 .split(QRegularExpression(QStringLiteral("[ 0-9/'\"\\(\\)&\\,.–„-]")), QString::SkipEmptyParts);
        std::for_each(l.begin(), l.end(), [](QString &s) {
            s = s.toCaseFolded();
        });
        l.removeAll(it.value().iataCode.toCaseFolded());
        l.removeAll(it.value().icaoCode.toCaseFolded());
        stripAirportAllLanguages(l);
        l.removeDuplicates();
        for (const auto &s : l) {
            if (s.size() <= 2) {
                continue;
            }
            labelMap[s].push_back(it.value().iataCode);
        }
    }
    for (auto it = labelMap.begin(); it != labelMap.end(); ++it) {
        std::sort(it.value().begin(), it.value().end());
    }

    // step 4 generate code
    CodeGen::writeLicenseHeader(out);
    out->write(R"(

#include "airportdb.h"
#include "airportdb_p.h"
#include "knowledgedb.h"
#include "timezonedb.h"

#include <limits>

using namespace KItinerary::KnowledgeDb;

namespace KItinerary {
namespace AirportDb {

// sorted table of all iata codes
// the corresponding index is used to acces data from all following tables
static constexpr IataCode iata_table[] = {
)");

    // IATA to airport data index
    for (auto it = m_iataMap.constBegin(); it != m_iataMap.constEnd(); ++it) {
        out->write("    IataCode{\"");
        out->write(it.key().toUtf8());
        out->write("\"}, // ");
        out->write(m_airportMap.value(it.value()).label.toUtf8());
        out->write("\n");
    }
    out->write(R"(};

// airport coordinates in latitude/longitude pairs
// NAN indicates the coordinate is not known
static constexpr Coordinate coordinate_table[] = {
)");
    // airport data tables - coordinates
    // TODO: should be possible to squeeze into 48 bit per coordinate, as 10m resolution is good enough for us
    for (auto it = m_iataMap.constBegin(); it != m_iataMap.constEnd(); ++it) {
        const auto &airport = m_airportMap.value(it.value());
        out->write("    ");
        CodeGen::writeCoordinate(out, airport.coord);
        out->write(", // ");
        out->write(airport.iataCode.toUtf8());
        out->write("\n");
    }
    out->write(R"(};

// timezone name string table indexes
static const Timezone timezone_table[] = {
)");
    for (auto it = m_iataMap.constBegin(); it != m_iataMap.constEnd(); ++it) {
        const auto &airport = m_airportMap.value(it.value());
        out->write("    ");
        CodeGen::writeTimezone(out, &tzDb, airport.tz);
        out->write(", // ");
        out->write(airport.iataCode.toUtf8());
        if (!airport.tz.isEmpty()) {
            out->write(" ");
            out->write(airport.tz);
        }
        out->write("\n");
    }
    out->write(R"(};

// reverse name lookup string table for unique strings
static const char name1_string_table[] =
)");
    // TODO prefix compression
    std::vector<Name1Index> string_offsets;
    string_offsets.reserve(labelMap.size());
    uint32_t label_offset = 0;
    for (auto it = labelMap.begin(); it != labelMap.end(); ++it) {
        if (it.value().size() > 1) {
            continue;
        }
        out->write("    \"");
        out->write(it.key().toUtf8());
        out->write("\" // ");
        out->write(it.value().at(0).toUtf8());
        out->write("\n");
        string_offsets.push_back(Name1Index{label_offset, (uint8_t)it.key().toUtf8().size(), (uint16_t)std::distance(m_iataMap.begin(), m_iataMap.find(it.value().at(0)))});
        label_offset += it.key().toUtf8().size();
    }
    out->write(R"(;

// string table indices into name_string_table
static const Name1Index name1_string_index[] = {
)");
    for (const auto &offset : string_offsets) {
        out->write("    Name1Index{");
        out->write(QByteArray::number(offset.offset()));
        out->write(", ");
        out->write(QByteArray::number(offset.length));
        out->write(", ");
        out->write(QByteArray::number(offset.iataIndex));
        out->write("},\n");
    }
    out->write(R"(};

// reverse name lookup string table for non-unique strings
static const char nameN_string_table[] =
)");
    // TODO prefix compression?
    struct stringN_index_t {
        QByteArray str;
        uint16_t strOffset;
        uint16_t iataMapOffset;
        QVector<QString> iataList;
    };
    std::vector<stringN_index_t> stringN_offsets;
    stringN_offsets.reserve(labelMap.size() - string_offsets.size());
    uint16_t string_offset = 0;
    uint16_t iata_map_offset = 0;
    for (auto it = labelMap.begin(); it != labelMap.end(); ++it) {
        if (it.value().size() == 1) {
            continue;
        }
        out->write("    \"");
        out->write(it.key().toUtf8());
        out->write("\"\n");
        stringN_offsets.emplace_back(stringN_index_t{it.key().toUtf8(), string_offset, iata_map_offset, it.value()});
        string_offset += it.key().toUtf8().size();
        iata_map_offset += it.value().size();
    }
    out->write(R"(;

// string table index to iata code mapping
static const uint16_t nameN_iata_table[] = {
)");
    for (const auto &offset : stringN_offsets) {
        out->write("    ");
        for (const auto &iataCode : offset.iataList) {
            out->write(QByteArray::number(std::distance(m_iataMap.begin(), m_iataMap.find(iataCode))));
            out->write(", ");
        }
        out->write(" // ");
        out->write(offset.str);
        out->write("\n");
    }
    out->write(R"(};

// index into the above string and iata index tables
static const NameNIndex nameN_string_index[] = {
)");
    for (const auto &offset : stringN_offsets) {
        out->write("    NameNIndex{");
        out->write(QByteArray::number(offset.strOffset));
        out->write(", ");
        out->write(QByteArray::number(offset.str.length()));
        out->write(", ");
        out->write(QByteArray::number(offset.iataMapOffset));
        out->write(", ");
        out->write(QByteArray::number(offset.iataList.size()));
        out->write("},\n");
    }
    out->write(R"(};
}
}
)");

    qDebug() << "Generated database containing" << m_iataMap.size() << "airports";
    qDebug() << "Name fragment index:" << string_offsets.size() << "unique keys," << labelMap.size() - string_offsets.size() << "non-unique keys";
    qDebug() << "IATA code collisions:" << m_iataCollisions;
    qDebug() << "Coordinate conflicts:" << m_coordinateConflicts;
    qDebug() << "Failed timezone lookups:" << m_timezoneLoopupFails;
}
