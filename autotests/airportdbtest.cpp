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

#include <KItinerary/AirportDb>

#include <QDebug>
#include <QObject>
#include <QTest>
#include <QTimeZone>

#include <cmath>

using namespace KItinerary;

class AirportDbTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void iataCodeTest()
    {
        const auto txl = KnowledgeDb::IataCode{"TXL"};
        QVERIFY(txl.isValid());
        const auto invalid = KnowledgeDb::IataCode{};
        QVERIFY(!invalid.isValid());
        QVERIFY(txl != invalid);
        QVERIFY(!(txl == invalid));
        QVERIFY(txl == txl);
        QCOMPARE(invalid.toString(), QString());

        const auto cdg = KnowledgeDb::IataCode{"CDG"};
        QVERIFY(cdg.isValid());
        QVERIFY(cdg != txl);
        QVERIFY(!(cdg == txl));
        QVERIFY(cdg < txl);
        QVERIFY(!(txl < cdg));

        QVERIFY(KnowledgeDb::IataCode{"ABC"} < KnowledgeDb::IataCode{"CBA"});
        QVERIFY(!(KnowledgeDb::IataCode{"CBA"} < KnowledgeDb::IataCode{"ABC"}));
    }

    void coordinateLookupTest()
    {
        auto coord = KnowledgeDb::coordinateForAirport(KnowledgeDb::IataCode{"TXL"});
        QVERIFY(coord.isValid());
        QCOMPARE((int)coord.longitude, 13);
        QCOMPARE((int)coord.latitude, 52);

        coord = KnowledgeDb::coordinateForAirport(KnowledgeDb::IataCode{"XXX"});
        QVERIFY(!coord.isValid());
        QVERIFY(std::isnan(coord.latitude));
        QVERIFY(std::isnan(coord.longitude));

        // test coordinate parsing corner cases
        coord = KnowledgeDb::coordinateForAirport(KnowledgeDb::IataCode{"LCY"});
        QCOMPARE((int)coord.longitude, 0);
        QVERIFY(coord.longitude > 0.0f);
        coord = KnowledgeDb::coordinateForAirport(KnowledgeDb::IataCode{"LHR"});
        QCOMPARE((int)coord.longitude, 0);
        QVERIFY(coord.longitude < 0.0f);

        // Köln-Bonn is a hybrid civilian/military airport, so that should be included
        coord = KnowledgeDb::coordinateForAirport(KnowledgeDb::IataCode{"CGN"});
        QVERIFY(coord.isValid());
        // Frankfurt-Hahn is a former military airport, should be included
        coord = KnowledgeDb::coordinateForAirport(KnowledgeDb::IataCode{"HHN"});
        QVERIFY(coord.isValid());
        // Ramstein is a military airport that should not be included
        coord = KnowledgeDb::coordinateForAirport(KnowledgeDb::IataCode{"RMS"});
        QVERIFY(!coord.isValid());

        // IATA codes that changed airports in various ways
        QVERIFY(KnowledgeDb::coordinateForAirport(KnowledgeDb::IataCode{"DEN"}).isValid());
        QVERIFY(KnowledgeDb::coordinateForAirport(KnowledgeDb::IataCode{"MUC"}).isValid());
        QVERIFY(KnowledgeDb::coordinateForAirport(KnowledgeDb::IataCode{"GOT"}).isValid());
        QVERIFY(KnowledgeDb::coordinateForAirport(KnowledgeDb::IataCode{"OSL"}).isValid());

        // IATA codes of no longer active airports
        QVERIFY(!KnowledgeDb::coordinateForAirport(KnowledgeDb::IataCode{"THF"}).isValid());

        // IATA codes of civilian airports that match the primitive military filter
        QVERIFY(KnowledgeDb::coordinateForAirport(KnowledgeDb::IataCode{"RAF"}).isValid());
        QVERIFY(KnowledgeDb::coordinateForAirport(KnowledgeDb::IataCode{"CFB"}).isValid());
        QVERIFY(KnowledgeDb::coordinateForAirport(KnowledgeDb::IataCode{"PAF"}).isValid());

        // one airport with 3 IATA codes
        coord = KnowledgeDb::coordinateForAirport(KnowledgeDb::IataCode{"BSL"});
        QVERIFY(coord.isValid());
        QCOMPARE(KnowledgeDb::coordinateForAirport(KnowledgeDb::IataCode{"BSL"}), KnowledgeDb::coordinateForAirport(KnowledgeDb::IataCode{"MLH"}));
        QCOMPARE(KnowledgeDb::coordinateForAirport(KnowledgeDb::IataCode{"BSL"}), KnowledgeDb::coordinateForAirport(KnowledgeDb::IataCode{"EAP"}));
    }

    void timezoneLookupTest()
    {
        auto tz = KnowledgeDb::timezoneForAirport(KnowledgeDb::IataCode{"TXL"});
        QVERIFY(tz.isValid());
        QCOMPARE(tz.id(), QByteArray("Europe/Berlin"));

        tz = KnowledgeDb::timezoneForAirport(KnowledgeDb::IataCode{"XXX"});
        QVERIFY(!tz.isValid());

        // tiny, make sure our lookup resolution is big enough for that
        tz = KnowledgeDb::timezoneForAirport(KnowledgeDb::IataCode{"LUX"});
        QCOMPARE(tz.id(), QByteArray("Europe/Luxembourg"));
    }

    void iataLookupTest()
    {
        // via unique fragment lookup
        QCOMPARE(KnowledgeDb::iataCodeFromName(QStringLiteral("Flughafen Berlin-Tegel")), KnowledgeDb::IataCode{"TXL"});
        QCOMPARE(KnowledgeDb::iataCodeFromName(QStringLiteral("TEGEL")), KnowledgeDb::IataCode{"TXL"});
        QCOMPARE(KnowledgeDb::iataCodeFromName(QStringLiteral("Paris Charles de Gaulle")), KnowledgeDb::IataCode{"CDG"});
        QCOMPARE(KnowledgeDb::iataCodeFromName(QStringLiteral("Zürich")), KnowledgeDb::IataCode{"ZRH"});
        QCOMPARE(KnowledgeDb::iataCodeFromName(QStringLiteral("AMSTERDAM, NL (SCHIPHOL AIRPORT)")), KnowledgeDb::IataCode{"AMS"});
        QCOMPARE(KnowledgeDb::iataCodeFromName(QStringLiteral("London Heathrow")), KnowledgeDb::IataCode{"LHR"});

        // via non-unique fragment lookup
        QCOMPARE(KnowledgeDb::iataCodeFromName(QStringLiteral("John F. Kennedy International Airport")), KnowledgeDb::IataCode{"JFK"});
        QCOMPARE(KnowledgeDb::iataCodeFromName(QStringLiteral("San Francisco International")), KnowledgeDb::IataCode{"SFO"});
        QCOMPARE(KnowledgeDb::iataCodeFromName(QStringLiteral("Düsseldorf International")), KnowledgeDb::IataCode{"DUS"});
        QCOMPARE(KnowledgeDb::iataCodeFromName(QStringLiteral("London City")), KnowledgeDb::IataCode{"LCY"});
        QCOMPARE(KnowledgeDb::iataCodeFromName(QStringLiteral("DETROIT, MI (METROPOLITAN WAYNE CO)")), KnowledgeDb::IataCode{"DTW"});

        // not unique
        QVERIFY(!KnowledgeDb::iataCodeFromName(QStringLiteral("Flughafen Berlin")).isValid());
        QVERIFY(!KnowledgeDb::iataCodeFromName(QStringLiteral("Charles de Gaulle Orly")).isValid());
        QVERIFY(!KnowledgeDb::iataCodeFromName(QStringLiteral("Brussels Airport, BE")).isValid());
        QVERIFY(!KnowledgeDb::iataCodeFromName(QStringLiteral("Frankfurt")).isValid());

        // string normalization
        QCOMPARE(KnowledgeDb::iataCodeFromName(QStringLiteral("Sao Paulo-Guarulhos International")), KnowledgeDb::IataCode{"GRU"});
        QCOMPARE(KnowledgeDb::iataCodeFromName(QStringLiteral("São Paulo-Guarulhos International")), KnowledgeDb::IataCode{"GRU"});
        QCOMPARE(KnowledgeDb::iataCodeFromName(QStringLiteral("Zurich")), KnowledgeDb::IataCode{"ZRH"});
        QCOMPARE(KnowledgeDb::iataCodeFromName(QStringLiteral("Dusseldorf International")), KnowledgeDb::IataCode{"DUS"});
        QCOMPARE(KnowledgeDb::iataCodeFromName(QStringLiteral("Almeria")), KnowledgeDb::IataCode{"LEI"});
        QCOMPARE(KnowledgeDb::iataCodeFromName(QStringLiteral("ALMERÍA")), KnowledgeDb::IataCode{"LEI"});
        QCOMPARE(KnowledgeDb::iataCodeFromName(QStringLiteral("Keflavík")), KnowledgeDb::IataCode{"KEF"});
        QCOMPARE(KnowledgeDb::iataCodeFromName(QStringLiteral("Keflavik")), KnowledgeDb::IataCode{"KEF"});
    }

    void countryDataTest()
    {
        auto iso = KnowledgeDb::countryForAirport(KnowledgeDb::IataCode{});
        QVERIFY(!iso.isValid());

        iso = KnowledgeDb::countryForAirport(KnowledgeDb::IataCode{"TXL"});
        QCOMPARE(iso, KnowledgeDb::CountryId{"DE"});
    }
};

QTEST_APPLESS_MAIN(AirportDbTest)

#include "airportdbtest.moc"
