/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "countrydbgenerator.h"
#include "codegen.h"
#include "util.h"
#include "wikidata.h"

#include <QDebug>
#include <QIODevice>
#include <QJsonArray>
#include <QJsonObject>

using namespace KItinerary;
using namespace KItinerary::Generator;

namespace KItinerary {
namespace Generator {

static bool operator<(const CountryDbGenerator::Country &lhs, const CountryDbGenerator::Country &rhs)
{
    return lhs.uri < rhs.uri;
}
static bool operator<(const CountryDbGenerator::Country &lhs, const QUrl &rhs)
{
    return lhs.uri < rhs;
}

}
}

bool CountryDbGenerator::generate(QIODevice* out)
{
    if (!fetchCountryList() || !fetchDrivingDirections() || !fetchPowerPlugTypes() || !fetchUicCountryCodes()) {
        return false;
    }

    CodeGen::writeLicenseHeaderWikidata(out);
    out->write(R"(
#include "knowledgedb.h"
#include "countrydb_p.h"

namespace KItinerary {
namespace KnowledgeDb {
)");
    writeCountryTable(out);
    writeUicCodeTable(out);
    out->write(R"(
}
}
)");

    printSummary();
    return true;
}

bool CountryDbGenerator::fetchCountryList()
{
    const auto countryArray = WikiData::query(R"(
        SELECT DISTINCT ?country ?countryLabel ?isoCode ?demolished WHERE {
            { ?country wdt:P31/wdt:P279* wd:Q6256. } UNION { ?country p:P31 [ ps:P31 [ wdt:279* wd:Q6256 ] ]. }
            ?country p:P297 [ ps:P297 ?isoCode ].
            OPTIONAL { ?country wdt:P576 ?demolished. }
            SERVICE wikibase:label { bd:serviceParam wikibase:language "en". }
        } ORDER BY (?country))", "wikidata_country.json");
    if (countryArray.isEmpty()) {
        qWarning() << "Empty query result!";
        return false;
    }

    for (const auto &countryData : countryArray) {
        const auto countryObj = countryData.toObject();
        if (countryObj.contains(QLatin1StringView("demolished"))) {
          continue;
        }
        const auto uri = insertOrMerge(countryObj);

        const auto isoCode = countryObj.value(QLatin1StringView("isoCode"))
                                 .toObject()
                                 .value(QLatin1StringView("value"))
                                 .toString()
                                 .toUpper();
        if (isoCode.size() != 2 || !Util::containsOnlyLetters(isoCode)) {
            qWarning() << "ISO 3166-1 alpha 2 format violation" << isoCode << uri;
            continue;
        }

        const auto it = m_isoCodeMap.find(isoCode);
        if (it != m_isoCodeMap.end() && (*it).second != uri) {
            ++m_isoCodeConflicts;
            qWarning() << "Conflict on ISO 3166-1 alpha 2 id" << isoCode << uri << m_isoCodeMap[isoCode];
        } else {
            m_isoCodeMap[isoCode] = uri;
        }
    }

    return true;
}

bool CountryDbGenerator::fetchDrivingDirections()
{
    const auto countryArray = WikiData::query(R"(
        SELECT DISTINCT ?country ?drivingSide ?drivingSideEndTime WHERE {
            { ?country wdt:P31/wdt:P279* wd:Q6256. } UNION { ?country p:P31 [ ps:P31 [ wdt:279* wd:Q6256 ] ]. }
            ?country p:P1622 ?drivingSideStmt.
            ?drivingSideStmt ps:P1622 ?drivingSide.
            OPTIONAL { ?drivingSideStmt pq:P582 ?drivingSideEndTime. }
        } ORDER BY (?country))", "wikidata_country_driving_side.json");
    if (countryArray.isEmpty()) {
        qWarning() << "Empty query result!";
        return false;
    }

    for (const auto &countryData : countryArray) {
        const auto countryObj = countryData.toObject();
        insertOrMerge(countryObj);
    }

    return true;
}

bool CountryDbGenerator::fetchPowerPlugTypes()
{
    const auto countryArray = WikiData::query(R"(
        SELECT DISTINCT ?country ?plugType ?plugTypeEndTime WHERE {
            { ?country wdt:P31/wdt:P279* wd:Q6256. } UNION { ?country p:P31 [ ps:P31 [ wdt:279* wd:Q6256 ] ]. }
            ?country p:P2853 ?plugTypeStmt.
            ?plugTypeStmt ps:P2853 ?plugType.
            OPTIONAL { ?plugTypeStmt pq:P582 ?plugTypeEndTime. }
        } ORDER BY (?country))", "wikidata_country_power_plug_type.json");
    if (countryArray.isEmpty()) {
        qWarning() << "Empty query result!";
        return false;
    }

    for (const auto &countryData : countryArray) {
        const auto countryObj = countryData.toObject();
        insertOrMerge(countryObj);
    }

    return true;
}

bool Generator::CountryDbGenerator::fetchUicCountryCodes()
{
    const auto uicArray = WikiData::query(R"(
        SELECT DISTINCT ?country ?uicCode WHERE {
            ?country (wdt:P31/wdt:P279*) wd:Q6256.
            ?country wdt:P2982 ?uicCode.
    } ORDER BY (?country))", "wikidata_country_uic_code.json");
    if (uicArray.isEmpty()) {
        qWarning() << "Empty UIC code query result!";
        return false;
    }

    for (const auto &uicCodeData : uicArray) {
        const auto uicObj = uicCodeData.toObject();
        const auto uicCode = uicObj.value(QLatin1StringView("uicCode"))
                                 .toObject()
                                 .value(QLatin1StringView("value"))
                                 .toString()
                                 .toUShort();
        const auto uri = QUrl(uicObj.value(QLatin1StringView("country"))
                                  .toObject()
                                  .value(QLatin1StringView("value"))
                                  .toString());
        const auto it = std::find_if(m_countries.begin(), m_countries.end(), [uri](const auto &country) { return country.uri == uri; });
        if (it == m_countries.end()) {
            qWarning() << "UIC code" << uicCode << "refers to unknown country" << uri;
            continue;
        }
        m_uicCodeMap[uicCode] = (*it).isoCode;

    }
    return true;
}


QUrl CountryDbGenerator::insertOrMerge(const QJsonObject& obj)
{
    if (obj.isEmpty()) {
        return {};
    }

    Country c;
    c.uri = QUrl(obj.value(QLatin1StringView("country"))
                     .toObject()
                     .value(QLatin1StringView("value"))
                     .toString());
    c.name = obj.value(QLatin1StringView("countryLabel"))
                 .toObject()
                 .value(QLatin1StringView("value"))
                 .toString();
    if (!obj.contains(QLatin1StringView("drivingSideEndTime"))) {
      c.drivingSide = obj.value(QLatin1StringView("drivingSide"))
                          .toObject()
                          .value(QLatin1StringView("value"))
                          .toString();
    }
    if (!obj.contains(QLatin1StringView("plugTypeEndTime")) &&
        obj.contains(QLatin1StringView("plugType"))) {
      c.powerPlugTypes.insert(QUrl(obj.value(QLatin1StringView("plugType"))
                                       .toObject()
                                       .value(QLatin1StringView("value"))
                                       .toString())
                                  .fileName());
    }
    c.isoCode = obj.value(QLatin1StringView("isoCode"))
                    .toObject()
                    .value(QLatin1StringView("value"))
                    .toString();

    const auto it = std::lower_bound(m_countries.begin(), m_countries.end(), c);
    if (it != m_countries.end() && (*it).uri == c.uri) {
        if (!(*it).drivingSide.isEmpty() && !c.drivingSide.isEmpty() && (*it).drivingSide != c.drivingSide) {
            qWarning() << "Conflicting driving side information for" << c.name << c.uri;
        } else if (!c.drivingSide.isEmpty()) {
            (*it).drivingSide = c.drivingSide;
        }
        (*it).powerPlugTypes += c.powerPlugTypes;
        return c.uri;
    }

    m_countries.insert(it, c);
    return c.uri;
}

struct plug_type_mapping {
    const char *wikidataId;
    const char *enumName;
};
static const plug_type_mapping plug_type_table[] = {
    { "Q24288454", "TypeA" },
    { "Q24288456", "TypeB" },
    { "Q1378312",  "TypeC" },
    { "Q60740126",  "TypeD" },
    { "Q2335536",  "TypeE" },
    { "Q1123613",  "TypeF" },
    { "Q1528507",  "TypeG" },
    { "Q1266396",  "TypeH" },
    { "Q2335539",  "TypeI" },
    { "Q2335530",  "TypeJ" },
    { "Q1502017",  "TypeK" },
    { "Q1520890",  "TypeL" },
    { "Q1383497",  "TypeM" },
    { "Q1653438",  "TypeN" }
};

static const auto plug_type_table_size = sizeof(plug_type_table) / sizeof(plug_type_mapping);

void CountryDbGenerator::writeCountryTable(QIODevice *out)
{
    const auto plug_type_table_end = plug_type_table + plug_type_table_size;

    out->write("static const Country country_table[] = {\n");
    for (const auto &kv : m_isoCodeMap) {
        const auto countryIt = std::lower_bound(m_countries.begin(), m_countries.end(), kv.second);
        out->write("    {CountryId{\"");
        out->write(kv.first.toLatin1());
        out->write("\"}, ");

        if ((*countryIt)
                .drivingSide.endsWith(QLatin1StringView("/Q14565199"))) {
          out->write("DrivingSide::Right");
        } else if ((*countryIt)
                       .drivingSide.endsWith(QLatin1StringView("/Q13196750"))) {
          out->write("DrivingSide::Left");
        } else {
          out->write("DrivingSide::Unknown");
        }

        out->write(", {");
        QStringList plugTypes;
        plugTypes.reserve((*countryIt).powerPlugTypes.size());
        for (const auto &plugType : std::as_const((*countryIt).powerPlugTypes)) {
          const auto it = std::find_if(
              plug_type_table, plug_type_table_end,
              [plugType](const plug_type_mapping &elem) {
                return QLatin1StringView(elem.wikidataId) == plugType;
              });
          if (it != plug_type_table_end) {
            plugTypes.push_back(QLatin1StringView((*it).enumName));
            } else {
                qWarning() << "Unknown plug type" << plugType << (*countryIt).name;
            }
        }
        std::sort(plugTypes.begin(), plugTypes.end());
        out->write(plugTypes.join(QLatin1Char('|')).toUtf8());
        out->write("}");

        out->write("}, // ");
        out->write((*countryIt).name.toUtf8());
        out->write("\n");
    }
    out->write("};\n\n");
}

void Generator::CountryDbGenerator::writeUicCodeTable(QIODevice *out)
{
    out->write("static const UicCountryCodeMapping uic_country_code_table[] = {\n");
    for (const auto &kv : m_uicCodeMap) {
        out->write("    {");
        out->write(QByteArray::number(kv.first));
        out->write(", CountryId{\"");
        out->write(kv.second.toUtf8());
        out->write("\"}},\n");
    }
    out->write("};\n\n");
}

void CountryDbGenerator::printSummary()
{
    qDebug() << "Generated database containing" << m_isoCodeMap.size() << "countries.";
    qDebug() << "Generated UIC code lookup table for" << m_uicCodeMap.size() << "countries.";
    qDebug() << "ISO 3166-1 alpha 2 code collisions:" << m_isoCodeConflicts;
}
