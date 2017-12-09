/*
   Copyright (c) 2017 Volker Krause <vkrause@kde.org>

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   This library is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
   License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.
*/

#include "timezones.h"
#include "airportdb_p.h"

#include <QByteArray>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QUrlQuery>

#include <cmath>

using namespace AirportDb;

struct Airport
{
    QUrl uri;
    QString iataCode;
    QString icaoCode;
    QString label;
    QString alias;
    QByteArray tz;
    int tzOffset;
    float longitude = NAN;
    float latitude = NAN;
};

static void parseCoordinate(const QString &value, Airport &a)
{
    const auto idx = value.indexOf(QLatin1Char(' '));
    bool latOk = false, longOk = false;
    a.longitude = value.midRef(6, idx - 6).toFloat(&latOk);
    a.latitude = value.midRef(idx + 1, value.size() - idx - 2).toFloat(&longOk);
    if (!latOk || !longOk) {
        a.longitude = NAN;
        a.latitude = NAN;
    }
}

static QJsonArray queryWikidata(const char *sparqlQuery, const QString &debugFileName)
{
    QUrl url(QStringLiteral("https://query.wikidata.org/sparql"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("query"), QString::fromUtf8(sparqlQuery).trimmed().simplified());
    query.addQueryItem(QStringLiteral("format"), QStringLiteral("json"));
    url.setQuery(query);

    QNetworkAccessManager nam;
    auto reply = nam.get(QNetworkRequest(url));
    QObject::connect(reply, &QNetworkReply::finished, QCoreApplication::instance(), &QCoreApplication::quit);
    QCoreApplication::instance()->exec();
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << reply->errorString();
        return {};
    }

    const auto data = reply->readAll();
    {
        QFile f(debugFileName);
        f.open(QFile::WriteOnly);
        f.write(data);
    }
    const auto doc = QJsonDocument::fromJson(data);
    const auto resultArray = doc.object().value(QLatin1String("results")).toObject().value(QLatin1String("bindings")).toArray();
    return resultArray;
}

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

int coordinateConflicts = 0;

static void merge(Airport &lhs, const Airport &rhs)
{
    if (lhs.iataCode != rhs.iataCode)
        qWarning() << "Multiple IATA codes on" << lhs.uri; // this can actually be valid, see BSL/MLH/EAP
    // we don't really care about multiple ICAO codes
//     if (lhs.icaoCode != rhs.icaoCode)
//         qWarning() << "Multiple ICAO codes on" << lhs.uri;
    QString extraLabel;
    if (lhs.label != rhs.label)
        extraLabel += QLatin1Char(' ') + rhs.label;
    if (lhs.alias != rhs.alias)
        extraLabel += QLatin1Char(' ') + rhs.alias;
    lhs.alias += extraLabel;
    if (std::isnan(lhs.latitude) || std::isnan(lhs.longitude)) {
        lhs.latitude = rhs.latitude;
        lhs.longitude = rhs.longitude;
    } else if (!std::isnan(rhs.latitude) && !std::isnan(rhs.longitude)) {
        if (std::abs(lhs.latitude - rhs.latitude) > 0.2f || std::abs(lhs.longitude - rhs.longitude) > 0.2f) {
            ++coordinateConflicts;
            qDebug() << lhs.uri << "has multiple conflicting coordinates";
        }
        // pick always the same independent of the input order, so stabilize generated output
        lhs.latitude = std::min(lhs.latitude, rhs.latitude);
        lhs.longitude = std::min(lhs.longitude, rhs.longitude);
    }
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QCommandLineParser parser;
    QCommandLineOption outputOpt({QStringLiteral("o"), QStringLiteral("output")}, QStringLiteral("Output file."), QStringLiteral("output file"));
    parser.addOption(outputOpt);
    parser.process(app);

    // step 1 query wikidata for all airports
    QUrl url(QStringLiteral("https://query.wikidata.org/sparql"));
    QUrlQuery query;
    // sorted by URI to stabilize the result in case of conflicts
    const auto airportArray = queryWikidata(R"(
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
        } ORDER BY (?airport))", QStringLiteral("wikidata_airports.json"));
    if (airportArray.isEmpty()) {
        qWarning() << "No results in SPARQL query found.";
        return 1;
    }

    QHash<QUrl, Airport> airportMap;
    QMap<QString, QUrl> iataMap;
    int iataCollisions = 0;
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
        // primitive military airport filter, turns out to be more reliable than the query below
        if (soundsMilitaryish(a.label) || soundsMilitaryish(a.alias))
            continue;
        parseCoordinate(obj.value(QLatin1String("coord")).toObject().value(QLatin1String("value")).toString(), a);

        // merge multiple records for the same airport
        auto it = airportMap.find(a.uri);
        if (it != airportMap.end()) {
            merge(*it, a);
            // continue nevertheless, to deal with multiple IATA codes per airport (e.g. BSL/MLH/EAP)
        } else {
            airportMap.insert(a.uri, a);
        }

        // TODO deal with IATA code duplications
        if (iataMap.contains(a.iataCode) && iataMap.value(a.iataCode) != a.uri) {
            ++iataCollisions;
            qDebug() << "duplicate iata code:" << a.iataCode << a.label << airportMap.value(iataMap.value(a.iataCode)).label;
        }
        iataMap.insert(a.iataCode, a.uri);
    }

    // step 1a query all military airports, and exlcude those
    // this could possibly be done with a single SPARQL query, but I haven't found one that works
    // within the 60s query timeout of Wikidata
    // TODO check for p:582 (endTime) on P31 (iow: exclude former military airports, such as HHN)
    const auto militaryAirportArray = queryWikidata(R"(
        SELECT DISTINCT ?airport ?iataCode WHERE {
            ?airport wdt:P31 wd:Q695850.
            ?airport wdt:P238 ?iataCode.
        })", QStringLiteral("wikidata_military_airports.json"));
    for (const auto &data : militaryAirportArray) {
        const auto uri = QUrl(data.toObject().value(QLatin1String("airport")).toObject().value(QLatin1String("value")).toString());
        const auto iataCode = data.toObject().value(QLatin1String("iataCode")).toObject().value(QLatin1String("value")).toString();
        // disabled for now, too many false positivies, e.g. CGN
//         if (iataMap.contains(iataCode))
//             qDebug() << "Removing military airport" << iataCode << airportMap.value(iataMap.value(iataCode)).label;
//         iataMap.remove(iataCode);
//         airportMap.remove(uri);
    }

    // step 2 augment the data with timezones
    Timezones tzDb;
    std::vector<QByteArray> usedTimezones;
    int timezoneLoopupFails = 0;
    for (auto it = airportMap.begin(); it != airportMap.end(); ++it) {
        if (std::isnan((*it).latitude) || std::isnan((*it).longitude)) {
            continue;
        }
        (*it).tz = tzDb.timezoneForCoordinate((*it).longitude, (*it).latitude);
        if ((*it).tz.isEmpty()) {
            qDebug() << "Failed to find timezone for" << (*it).iataCode << (*it).latitude << (*it).longitude;
            ++timezoneLoopupFails;
            continue;
        }
        auto tzIt = std::lower_bound(usedTimezones.begin(), usedTimezones.end(), it.value().tz);
        if (tzIt == usedTimezones.end() || (*tzIt) != it.value().tz)
            usedTimezones.insert(tzIt, it.value().tz);
    }
    std::vector<uint16_t> timezoneOffsets;
    timezoneOffsets.reserve(usedTimezones.size());
    uint16_t offset = 0;
    for (const auto &tz : usedTimezones) {
        timezoneOffsets.push_back(offset);
        offset += tz.size() + 1; // +1 of the trailing null byte
    }

    // step 3 index the names for reverse lookup
    QMap<QString, QString> labelMap;
    for (auto it = airportMap.begin(); it != airportMap.end(); ++it) {
        auto l = (it.value().label + QLatin1Char(' ') + it.value().alias)
            .split(QRegularExpression(QStringLiteral("[ 0-9/'\"\\(\\)&\\,.–„-]")), QString::SkipEmptyParts);
        std::for_each(l.begin(), l.end(), [](QString &s) { s = s.toCaseFolded(); });
        l.removeAll(it.value().iataCode.toCaseFolded());
        l.removeAll(it.value().icaoCode.toCaseFolded());
        l.removeDuplicates();
        for (const auto &s : l) {
            if (s.size() <= 2)
                continue;
            if (!labelMap.contains(s)) {
                labelMap.insert(s, it.value().iataCode);
            } else {
//                 if (!labelMap.value(s).isEmpty())
//                     qDebug() << "clash on" << s;
                labelMap[s] = QString();
            }
        }
    }

    // step 4 generate code
    QFile f(parser.value(outputOpt));
    if (!f.open(QFile::WriteOnly)) {
        qWarning() << f.errorString();
        return 1;
    }
    f.write(R"(/*
 * This code is auto-generated from Wikidata data. Licensed under CC0.
 */

#include "airportdb.h"
#include "airportdb_p.h"

#include <cmath>
#include <limits>

namespace AirportDb {

// sorted table of all iata codes
// the corresponding index is used to acces data from all following tables
static constexpr IataCode iata_table[] = {
)");

    // IATA to airport data index
    for (auto it = iataMap.constBegin(); it != iataMap.constEnd(); ++it) {
        f.write("    IataCode{\"");
        f.write(it.key().toUtf8());
        f.write("\"}, // ");
        f.write(airportMap.value(it.value()).label.toUtf8());
        f.write("\n");
    }
    f.write(R"(};

// airport coordinates in latitude/longitude pairs
// NAN indicates the coordinate is not known
static constexpr Coordinate coordinate_table[] = {
)");
    // airport data tables - coordinates
    // TODO: should be possible to squeeze into 48 bit per coordinate, as 10m resolution is good enough for us
    for (auto it = iataMap.constBegin(); it != iataMap.constEnd(); ++it) {
        const auto &airport = airportMap.value(it.value());
        f.write("    Coordinate{");
        if (!std::isnan(airport.longitude) && !std::isnan(airport.latitude)) {
            f.write(QByteArray::number(airport.longitude));
            f.write(", ");
            f.write(QByteArray::number(airport.latitude));
        }
        f.write("}, // ");
        f.write(airport.iataCode.toUtf8());
        f.write("\n");
    }
    f.write(R"(};

// timezone name strings
static const char timezone_names[] =
)");
    // timezone string tables
    for (const auto &tz : usedTimezones) {
        f.write("    ");
        f.write("\"");
        f.write(tz);
        f.write("\\0\"\n");
    }
    f.write(R"(;

// timezone name string table indexes
static const uint16_t timezone_table[] = {
)");
    for (auto it = iataMap.constBegin(); it != iataMap.constEnd(); ++it) {
        const auto &airport = airportMap.value(it.value());
        f.write("    ");
        const auto tzIt = std::lower_bound(usedTimezones.begin(), usedTimezones.end(), airport.tz);
        if (tzIt != usedTimezones.end() && (*tzIt) == airport.tz) {
            const auto tzIdx = std::distance(usedTimezones.begin(), tzIt);
            f.write(QByteArray::number(timezoneOffsets[tzIdx]));
        } else {
            f.write("std::numeric_limits<uint16_t>::max()");
        }
        f.write(", // ");
        f.write(airport.iataCode.toUtf8());
        if (tzIt != usedTimezones.end() && (*tzIt) == airport.tz) {
            f.write(" ");
            f.write(*tzIt);
        }
        f.write("\n");
    }
    f.write(R"(};

// reverse name lookup string table
static const char name_string_table[] =
)");
    // TODO prefix compression
    std::vector<NameIndex> string_offsets;
    string_offsets.reserve(labelMap.size());
    uint32_t label_offset = 0;
    for (auto it = labelMap.begin(); it != labelMap.end(); ++it) {
        if (it.value().isEmpty())
            continue;
        f.write("    \"");
        f.write(it.key().toUtf8());
        f.write("\" // ");
        f.write(it.value().toUtf8());
        f.write("\n");
        string_offsets.push_back(NameIndex{label_offset, (uint8_t)it.key().toUtf8().size(), (uint16_t)std::distance(iataMap.begin(), iataMap.find(it.value()))});
        label_offset += it.key().toUtf8().size();
    }
    f.write(R"(;

// string table indices into name_string_table
static const NameIndex name_string_index[] = {
)");
    for (const auto &offset : string_offsets) {
        f.write("    NameIndex{");
        f.write(QByteArray::number(offset.offset()));
        f.write(", ");
        f.write(QByteArray::number(offset.length));
        f.write(", ");
        f.write(QByteArray::number(offset.iataIndex));
        f.write("},\n");
    }
    f.write(R"(};

}
)");

    qDebug() << "Generated database containing" << iataMap.size() << "airports and" << labelMap.size() << "name keys.";
    qDebug() << "IATA code collisions:" << iataCollisions;
    qDebug() << "Coordinate conflicts:" << coordinateConflicts;
    qDebug() << "Failed timezone lookups:" << timezoneLoopupFails;
    return 0;
}
