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
        const auto txl = AirportDb::IataCode{"TXL"};
        QVERIFY(txl.isValid());
        const auto invalid = AirportDb::IataCode{};
        QVERIFY(!invalid.isValid());
        QVERIFY(txl != invalid);
        QVERIFY(!(txl == invalid));
        QVERIFY(txl == txl);
        QCOMPARE(invalid.toString(), QString());

        const auto cdg = AirportDb::IataCode{"CDG"};
        QVERIFY(cdg.isValid());
        QVERIFY(cdg != txl);
        QVERIFY(!(cdg == txl));
        QVERIFY(cdg < txl);
        QVERIFY(!(txl < cdg));

        QVERIFY(AirportDb::IataCode{"ABC"} < AirportDb::IataCode{"CBA"});
        QVERIFY(!(AirportDb::IataCode{"CBA"} < AirportDb::IataCode{"ABC"}));
    }

    void coordinateLookupTest()
    {
        auto coord = AirportDb::coordinateForAirport(AirportDb::IataCode{"TXL"});
        QVERIFY(coord.isValid());
        QCOMPARE((int)coord.longitude, 13);
        QCOMPARE((int)coord.latitude, 52);

        coord = AirportDb::coordinateForAirport(AirportDb::IataCode{"XXX"});
        QVERIFY(!coord.isValid());
        QVERIFY(std::isnan(coord.latitude));
        QVERIFY(std::isnan(coord.longitude));

        // test coordinate parsing corner cases
        coord = AirportDb::coordinateForAirport(AirportDb::IataCode{"LCY"});
        QCOMPARE((int)coord.longitude, 0);
        QVERIFY(coord.longitude > 0.0f);
        coord = AirportDb::coordinateForAirport(AirportDb::IataCode{"LHR"});
        QCOMPARE((int)coord.longitude, 0);
        QVERIFY(coord.longitude < 0.0f);

        // Köln-Bonn is a hybrid civilian/military airport, so that should be included
        coord = AirportDb::coordinateForAirport(AirportDb::IataCode{"CGN"});
        QVERIFY(coord.isValid());
        // Frankfurt-Hahn is a former military airport, should be included
        coord = AirportDb::coordinateForAirport(AirportDb::IataCode{"HHN"});
        QVERIFY(coord.isValid());
        // Ramstein is a military airport that should not be included
        coord = AirportDb::coordinateForAirport(AirportDb::IataCode{"RMS"});
        QVERIFY(!coord.isValid());

        // IATA codes that changed airports in various ways
        QVERIFY(AirportDb::coordinateForAirport(AirportDb::IataCode{"DEN"}).isValid());
        QVERIFY(AirportDb::coordinateForAirport(AirportDb::IataCode{"MUC"}).isValid());
        QVERIFY(AirportDb::coordinateForAirport(AirportDb::IataCode{"GOT"}).isValid());
        QVERIFY(AirportDb::coordinateForAirport(AirportDb::IataCode{"OSL"}).isValid());

        // IATA codes of no longer active airports
        QVERIFY(!AirportDb::coordinateForAirport(AirportDb::IataCode{"THF"}).isValid());

        // IATA codes of civilian airports that match the primitive military filter
        QVERIFY(AirportDb::coordinateForAirport(AirportDb::IataCode{"RAF"}).isValid());
        QVERIFY(AirportDb::coordinateForAirport(AirportDb::IataCode{"CFB"}).isValid());
        QVERIFY(AirportDb::coordinateForAirport(AirportDb::IataCode{"PAF"}).isValid());

        // one airport with 3 IATA codes
        coord = AirportDb::coordinateForAirport(AirportDb::IataCode{"BSL"});
        QVERIFY(coord.isValid());
        QCOMPARE(AirportDb::coordinateForAirport(AirportDb::IataCode{"BSL"}), AirportDb::coordinateForAirport(AirportDb::IataCode{"MLH"}));
        QCOMPARE(AirportDb::coordinateForAirport(AirportDb::IataCode{"BSL"}), AirportDb::coordinateForAirport(AirportDb::IataCode{"EAP"}));
    }

    void timezoneLookupTest()
    {
        auto tz = AirportDb::timezoneForAirport(AirportDb::IataCode{"TXL"});
        QVERIFY(tz.isValid());
        QCOMPARE(tz.id(), QByteArray("Europe/Berlin"));

        tz = AirportDb::timezoneForAirport(AirportDb::IataCode{"XXX"});
        QVERIFY(!tz.isValid());

        // tiny, make sure our lookup resolution is big enough for that
        tz = AirportDb::timezoneForAirport(AirportDb::IataCode{"LUX"});
        QCOMPARE(tz.id(), QByteArray("Europe/Luxembourg"));
    }

    void iataLookupTest()
    {
        // via unique fragment lookup
        QCOMPARE(AirportDb::iataCodeFromName(QStringLiteral("Flughafen Berlin-Tegel")), AirportDb::IataCode{"TXL"});
        QCOMPARE(AirportDb::iataCodeFromName(QStringLiteral("TEGEL")), AirportDb::IataCode{"TXL"});
        QCOMPARE(AirportDb::iataCodeFromName(QStringLiteral("Paris Charles de Gaulle")), AirportDb::IataCode{"CDG"});
        QCOMPARE(AirportDb::iataCodeFromName(QStringLiteral("Zürich")), AirportDb::IataCode{"ZRH"});
        QCOMPARE(AirportDb::iataCodeFromName(QStringLiteral("AMSTERDAM, NL (SCHIPHOL AIRPORT)")), AirportDb::IataCode{"AMS"});
        QCOMPARE(AirportDb::iataCodeFromName(QStringLiteral("London Heathrow")), AirportDb::IataCode{"LHR"});

        // via non-unique fragment lookup
        QCOMPARE(AirportDb::iataCodeFromName(QStringLiteral("John F. Kennedy International Airport")), AirportDb::IataCode{"JFK"});
        QCOMPARE(AirportDb::iataCodeFromName(QStringLiteral("San Francisco International")), AirportDb::IataCode{"SFO"});
        QCOMPARE(AirportDb::iataCodeFromName(QStringLiteral("Düsseldorf International")), AirportDb::IataCode{"DUS"});
        QCOMPARE(AirportDb::iataCodeFromName(QStringLiteral("London City")), AirportDb::IataCode{"LCY"});
        QCOMPARE(AirportDb::iataCodeFromName(QStringLiteral("DETROIT, MI (METROPOLITAN WAYNE CO)")), AirportDb::IataCode{"DTW"});

        // not unique
        QVERIFY(!AirportDb::iataCodeFromName(QStringLiteral("Flughafen Berlin")).isValid());
        QVERIFY(!AirportDb::iataCodeFromName(QStringLiteral("Charles de Gaulle Orly")).isValid());
        QVERIFY(!AirportDb::iataCodeFromName(QStringLiteral("Brussels Airport, BE")).isValid());
        QVERIFY(!AirportDb::iataCodeFromName(QStringLiteral("Frankfurt")).isValid());
    }

    void countryDataTest()
    {
        auto iso = AirportDb::countryForAirport(AirportDb::IataCode{});
        QVERIFY(!iso.isValid());

        iso = AirportDb::countryForAirport(AirportDb::IataCode{"TXL"});
        QCOMPARE(iso, KnowledgeDb::CountryId{"DE"});
    }
};

QTEST_APPLESS_MAIN(AirportDbTest)

#include "airportdbtest.moc"
