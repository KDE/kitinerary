/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "airportdbgenerator.h"
#include "codegen.h"
#include "wikidata.h"
#include "../stringutil.h"
#include "../knowledgedb/airportnametokenizer_p.h"

#include "airportdb_p.h"

#include <QDateTime>
#include <QDebug>
#include <QIODevice>
#include <QJsonArray>
#include <QJsonObject>
#include <QRegularExpression>

using namespace KItinerary;
using namespace KItinerary::KnowledgeDb;
using namespace KItinerary::Generator;

static bool soundsMilitaryish(const QString &s)
{
  return s.contains(QLatin1StringView("Airbase"), Qt::CaseInsensitive) ||
         s.contains(QLatin1StringView("Air Base"), Qt::CaseInsensitive) ||
         s.contains(QLatin1StringView("Air Force"), Qt::CaseInsensitive) ||
         s.contains(QLatin1StringView("Air National Guard Base"),
                    Qt::CaseInsensitive) ||
         s.contains(QLatin1StringView("Air Reserve Base"),
                    Qt::CaseInsensitive) ||
         s.contains(QLatin1StringView("Air Station"), Qt::CaseInsensitive) ||
         s.contains(QLatin1StringView("Army Airfield"), Qt::CaseInsensitive) ||
         s.contains(QLatin1StringView("Army Airport"), Qt::CaseInsensitive) ||
         s.contains(QLatin1StringView("Army Heliport"), Qt::CaseInsensitive) ||
         s.contains(QLatin1StringView("Canadian Forces Base"),
                    Qt::CaseInsensitive) ||
         s.contains(QLatin1StringView("Joint Base"), Qt::CaseInsensitive) ||
         s.contains(QLatin1StringView("Marine Corps"), Qt::CaseInsensitive) ||
         s.contains(QLatin1StringView("Military ")) ||
         s.contains(QLatin1StringView("Naval ")) ||
         s.contains(QLatin1StringView("RAF ")) ||
         s.contains(QLatin1StringView("RAAF ")) ||
         s.contains(QLatin1StringView("RNAS ")) ||
         s.contains(QLatin1StringView("CFB ")) ||
         s.contains(QLatin1StringView("PAF ")) ||
         s.contains(QLatin1StringView("NAF "));
}

static void stripAirportAllLanguages(QStringList &s)
{
    // only languages used in the English (sic!) wikidata labels and description matter here
    s.removeAll(QStringLiteral("aeroport"));
    s.removeAll(QStringLiteral("aeroporto"));
    s.removeAll(QStringLiteral("aeropuerto"));
    s.removeAll(QStringLiteral("air"));
    s.removeAll(QStringLiteral("airfield"));
    s.removeAll(QStringLiteral("airpark"));
    s.removeAll(QStringLiteral("airport"));
    s.removeAll(QStringLiteral("airstrip"));
    s.removeAll(QStringLiteral("flughafen"));
    s.removeAll(QStringLiteral("lufthavn"));
    s.removeAll(QStringLiteral("terminal"));
}

static void normalizeAbbreviations(QStringList &l)
{
    for (auto &s : l) {
      if (s == QLatin1StringView("intl")) {
        s = QStringLiteral("international");
      }
    }
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
            qDebug() << lhs.label << lhs.iataCode << lhs.uri << "has multiple conflicting coordinates";
        }
        // pick always the same independent of the input order, so stabilize generated output
        lhs.coord.latitude = std::min(lhs.coord.latitude, rhs.coord.latitude);
        lhs.coord.longitude = std::min(lhs.coord.longitude, rhs.coord.longitude);
    }
}

bool AirportDbGenerator::fetchAirports()
{
    // sorted by URI to stabilize the result in case of conflicts
    const auto airportArray = WikiData::query(R"(
        SELECT DISTINCT ?airport ?airportLabel ?airportAltLabel ?iataCode ?icaoCode ?coord ?endDate ?demolished ?officialClosure ?openingDate ?iataEndDate ?iataRank WHERE {
            ?airport (wdt:P31/wdt:P279*) wd:Q1248784.
            ?airport p:P238 ?iataStmt.
            ?iataStmt ps:P238 ?iataCode.
            OPTIONAL { ?airport wdt:P239 ?icaoCode. }
            OPTIONAL { ?airport wdt:P625 ?coord. }
            OPTIONAL { ?airport wdt:P582 ?endDate. }
            OPTIONAL { ?airport wdt:P576 ?demolished. }
            OPTIONAL { ?airport wdt:P3999 ?officialClosure. }
            OPTIONAL { ?airport wdt:P1619 ?openingDate. }
            OPTIONAL { ?iataStmt pq:P582 ?iataEndDate. }
            OPTIONAL { ?iataStmt wikibase:rank ?iataRank. }
            SERVICE wikibase:label { bd:serviceParam wikibase:language "en". }
        } ORDER BY (?airport))", "wikidata_airports.json");
    if (airportArray.isEmpty()) {
        qWarning() << "No results in SPARQL query found.";
        return false;
    }

    for (const auto &data: airportArray) {
        const auto obj = data.toObject();
        if (obj.isEmpty()) {
            continue;
        }

        if (obj.contains(QLatin1StringView("endDate")) ||
            obj.contains(QLatin1StringView("demolished")) ||
            obj.contains(QLatin1StringView("officialClosure")) ||
            obj.contains(QLatin1StringView("iataEndDate")) ||
            obj.value(QLatin1StringView("iataRank"))
                .toObject()
                .value(QLatin1StringView("value"))
                .toString()
                .endsWith(QLatin1StringView("DeprecatedRank"))) {
          // skip closed airports or those with expired IATA codes
          continue;
        }

        const auto openingdDt =
            QDateTime::fromString(obj.value(QLatin1StringView("openingDate"))
                                      .toObject()
                                      .value(QLatin1StringView("value"))
                                      .toString(),
                                  Qt::ISODate);
        if (openingdDt.isValid() && openingdDt > QDateTime::currentDateTime().addDays(120)) {
            // skip future airports
            continue;
        }

        Airport a;
        a.uri = QUrl(obj.value(QLatin1StringView("airport"))
                         .toObject()
                         .value(QLatin1StringView("value"))
                         .toString());
        a.iataCode = obj.value(QLatin1StringView("iataCode"))
                         .toObject()
                         .value(QLatin1StringView("value"))
                         .toString();
        if (a.iataCode.size() != 3 || !a.iataCode.at(0).isUpper() || !a.iataCode.at(1).isUpper() || !a.iataCode.at(2).isUpper()) {
            // invalid IATA code
            continue;
        }
        a.icaoCode = obj.value(QLatin1StringView("icaoCode"))
                         .toObject()
                         .value(QLatin1StringView("value"))
                         .toString();
        a.label = obj.value(QLatin1StringView("airportLabel"))
                      .toObject()
                      .value(QLatin1StringView("value"))
                      .toString();
        a.alias = obj.value(QLatin1StringView("airportAltLabel"))
                      .toObject()
                      .value(QLatin1StringView("value"))
                      .toString();
        // primitive military airport filter, turns out to be more reliable than querying for the military airport types
        if (soundsMilitaryish(a.label) || soundsMilitaryish(a.alias)) {
            continue;
        }
        a.coord =
            WikiData::parseCoordinate(obj.value(QLatin1StringView("coord"))
                                          .toObject()
                                          .value(QLatin1StringView("value"))
                                          .toString());

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

    return true;
}

bool AirportDbGenerator::fetchCountries()
{
    const auto array = WikiData::query(R"(
        SELECT DISTINCT ?airport ?isoCode WHERE {
            ?airport (wdt:P31/wdt:P279*) wd:Q1248784.
            ?airport wdt:P17 ?country.
            ?country wdt:P297 ?isoCode.
        } ORDER BY (?airport))", "wikidata_airport_country.json");
    if (array.isEmpty()) {
        qWarning() << "Empty query result!";
        return false;
    }

    for (const auto &airportData: array) {
        const auto obj = airportData.toObject();
        const auto uri = QUrl(obj.value(QLatin1StringView("airport"))
                                  .toObject()
                                  .value(QLatin1StringView("value"))
                                  .toString());
        const auto isoCode = obj.value(QLatin1StringView("isoCode"))
                                 .toObject()
                                 .value(QLatin1StringView("value"))
                                 .toString();
        const auto it = m_airportMap.find(uri);
        if (it != m_airportMap.end()) {
            if ((*it).country != isoCode && !(*it).country.isEmpty()) {
                ++m_countryConflicts;
                qWarning() << "Country code conflict on" << (*it).label << (*it).uri << (*it).country << isoCode;
                continue;
            }
            (*it).country = isoCode;
        }
    }

    return true;
}

void AirportDbGenerator::improveCoordinates()
{
    for (auto it = m_airportMap.begin(); it != m_airportMap.end(); ++it) {
        if (!(*it).coord.isValid()) {
            continue;
        }
        const auto coord = osmDb.lookup((*it).iataCode, (*it).coord.latitude, (*it).coord.longitude);
        if (coord.isValid()) {
            (*it).coord.latitude = coord.latF();
            (*it).coord.longitude = coord.lonF();
        }
    }
}

void AirportDbGenerator::indexNames()
{
    for (auto it = m_airportMap.begin(); it != m_airportMap.end(); ++it) {
        const auto normalized = StringUtil::normalize(QString(it.value().label + QLatin1Char(' ') + it.value().alias));
        AirportNameTokenizer tokenizer(normalized);
        auto l = tokenizer.toStringList();
        normalizeAbbreviations(l);
        std::sort(l.begin(), l.end());
        l.removeAll(it.value().iataCode.toCaseFolded());
        l.removeAll(it.value().icaoCode.toCaseFolded());
        stripAirportAllLanguages(l);
        l.erase(std::unique(l.begin(), l.end()), l.end());
        l.erase(std::remove_if(l.begin(), l.end(), [](const auto &s) { return s.size() <= 2; }), l.end());
        for (const auto &s : std::as_const(l)) {
            (*it).fragments.push_back(s);
            m_labelMap[s].push_back(it.value().iataCode);
        }
    }
    for (auto it = m_labelMap.begin(); it != m_labelMap.end();) {
        std::sort(it.value().begin(), it.value().end());

        // TODO revisit this when we have experience with the higher level IATA code disambiguation in FlightPostProcessor
#if 0
        if (it.value().size() == 2) { // TODO generalize this from 2 to N
            const auto lhsAirport = m_airportMap.value(m_iataMap.value(it.value().at(0)));
            const auto rhsAirport = m_airportMap.value(m_iataMap.value(it.value().at(1)));

            // if the fragments are exactly identical, this will never be unique
            // removing the current fragment from the index is fine, but we can't do that elsewhere if the collision set is > 2
            if (lhsAirport.fragments == rhsAirport.fragments) {
                qDebug() << "Unresolvable index collision for" << lhsAirport.iataCode << lhsAirport.label << lhsAirport.uri << rhsAirport.iataCode << rhsAirport.label << rhsAirport.uri;
                it = m_labelMap.erase(it);
                continue;
            }

            // TODO check if one fragment set is a subset of the other, in which case the set difference
            // can be used for the exclusion index
            std::vector<QString> leftDiff, rightDiff;
            std::set_difference(lhsAirport.fragments.begin(), lhsAirport.fragments.end(), rhsAirport.fragments.begin(), rhsAirport.fragments.end(), std::back_inserter(leftDiff));
            std::set_difference(rhsAirport.fragments.begin(), rhsAirport.fragments.end(), lhsAirport.fragments.begin(), lhsAirport.fragments.end(), std::back_inserter(rightDiff));
            if (leftDiff.empty() || rightDiff.empty()) {
                qDebug() << "Subset fragments:" << leftDiff << rightDiff << lhsAirport.fragments << rhsAirport.fragments;
            } else {
                //qDebug() << "Overlapping fragments:" << lhsAirport.fragments << rhsAirport.fragments;
            }
        }
#endif

        ++it;
    }
}


bool AirportDbGenerator::generate(QIODevice* out)
{
    // step 1 query wikidata for all airports
    if (!fetchAirports() || !fetchCountries()) {
        return false;
    }

    // step 2 augment the data with optimized OSM airport positions
    improveCoordinates();

    // step 3 index the names for reverse lookup
    indexNames();

    // step 4 generate code
    CodeGen::writeLicenseHeaderOSM(out);
    out->write(R"(

#include "airportdb.h"
#include "airportdb_p.h"
#include "knowledgedb.h"

#include <limits>

using namespace KItinerary::KnowledgeDb;

namespace KItinerary {
namespace KnowledgeDb {

// airport data sorted by IATA code
// the corresponding index is used to access data the following tables
static constexpr Airport airport_table[] = {
)");

    // IATA to airport data index
    for (auto it = m_iataMap.constBegin(); it != m_iataMap.constEnd(); ++it) {
        out->write("    Airport{IataCode{\"");
        out->write(it.key().toUtf8());
        out->write("\"}, ");
        CodeGen::writeCountryIsoCode(out, m_airportMap.value(it.value()).country);
        out->write(", ");
        CodeGen::writeCoordinate(out, m_airportMap.value(it.value()).coord);
        out->write("}, // ");
        out->write(m_airportMap.value(it.value()).label.toUtf8());
        out->write("\n");
    }
    out->write(R"(};

// reverse name lookup string table for unique strings
static const char name1_string_table_0[] =
)");
    // TODO suffix compression -> see KPublicTransport, that has generic code for this
    std::vector<Name1Index> string_offsets;
    string_offsets.reserve(m_labelMap.size());
    uint32_t label_offset = 0;
    uint32_t batch_size = 0;
    for (auto it = m_labelMap.constBegin(); it != m_labelMap.constEnd(); ++it) {
        if (it.value().size() > 1) {
            continue;
        }

        const auto keySize = it.key().toUtf8().size();
        if (batch_size + keySize > (1 << 16)) { // max string size on MSVC...
            out->write(R"(;
static const char name1_string_table_1[] =
)");
            batch_size = 0;
        }

        out->write("    \"");
        out->write(it.key().toUtf8());
        out->write("\" // ");
        out->write(it.value().at(0).toUtf8());
        out->write("\n");
        string_offsets.emplace_back(Name1Index{label_offset, (uint8_t)keySize, (uint16_t)std::distance(m_iataMap.begin(), m_iataMap.find(it.value().at(0)))});
        label_offset += keySize;
        batch_size += keySize;
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
        QList<QString> iataList;
    };
    std::vector<stringN_index_t> stringN_offsets;
    stringN_offsets.reserve(m_labelMap.size() - string_offsets.size());
    uint16_t string_offset = 0;
    uint16_t iata_map_offset = 0;
    for (auto it = m_labelMap.constBegin(); it != m_labelMap.constEnd(); ++it) {
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
            out->write(QByteArray::number(std::distance(m_iataMap.constBegin(), m_iataMap.constFind(iataCode))));
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
    qDebug() << "Name fragment index:" << string_offsets.size() << "unique keys," << m_labelMap.size() - string_offsets.size() << "non-unique keys";
    qDebug() << "IATA code collisions:" << m_iataCollisions;
    qDebug() << "Coordinate conflicts:" << m_coordinateConflicts;
    qDebug() << "Country conflicts:" << m_countryConflicts;
    qDebug() << "Failed timezone lookups:" << m_timezoneLoopupFails;

    return true;
}
