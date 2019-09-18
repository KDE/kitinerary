/*
  Copyright (c) 2018 Volker Krause <vkrause@kde.org>

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

#include <KItinerary/CountryDb>

#include <knowledgedb/alphaid.h>
#include <knowledgedb/trainstationdb.h>

#include <QDebug>
#include <QObject>
#include <QTest>
#include <QTimeZone>

using namespace KItinerary;
using namespace KItinerary::KnowledgeDb;

class KnowledgeDbTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testUnalignedNumber()
    {
        constexpr UnalignedNumber<3> uic1(8001337);
        constexpr UnalignedNumber<3> uic2(8001330);
        static_assert(sizeof(uic1) == 3, "");
        static_assert(alignof(UnalignedNumber<3>) == 1, "");
        QVERIFY(!(uic1 < uic2));
        QVERIFY(uic2 < uic1);
        QVERIFY(uic1 != uic2);
        QVERIFY(!(uic1 == uic2));

        constexpr UnalignedNumber<3> uic3(9899776);
        constexpr UnalignedNumber<3> uic4(1000191);
        QVERIFY(!(uic3 < uic4));
        QVERIFY(uic4 < uic3);
        QVERIFY(uic3 != uic4);
        QVERIFY(!(uic3 == uic4));
        QCOMPARE(uic3.value(), 9899776);
        QCOMPARE(uic4.value(), 1000191);

        constexpr UnalignedNumber<3> uic[2] = { UnalignedNumber<3>(8301700), UnalignedNumber<3>(8301701) };
        static_assert(sizeof(uic) == 6, "");
        QVERIFY(uic[0] < uic[1]);
        QVERIFY(uic[0] == uic[0]);
        QVERIFY(uic[0] != uic[1]);
    }

   void testAlphaId()
   {
       using ID3 = AlphaId<uint16_t, 3>;
       constexpr ID3 id1{"ABC"};
       const ID3 id2(QStringLiteral("CBA"));
       static_assert(sizeof(id1) == 2, "");
       QVERIFY(id1.isValid());
       QVERIFY(id2.isValid());
       QVERIFY(id1 < id2);
       QVERIFY(!(id2 < id1));
       QVERIFY(id1 == id1);
       QVERIFY(id1 != id2);
       QVERIFY(!(id1 == id2));
       QVERIFY(!(id1 != id1));

       QCOMPARE(id1.toString(), QLatin1String("ABC"));
       QCOMPARE(id2.toString(), QLatin1String("CBA"));

       constexpr ID3 id3;
       QVERIFY(!id3.isValid());
       QVERIFY(id3.toString().isEmpty());

       qDebug() << id1;
    }

    void testIBNRLookup()
    {
        auto station = KnowledgeDb::stationForIbnr(IBNR{1234567});
        QVERIFY(!station.coordinate.isValid());
        QCOMPARE(station.timezone.toQTimeZone(), QTimeZone());

        station = KnowledgeDb::stationForIbnr({});
        QVERIFY(!station.coordinate.isValid());
        QCOMPARE(station.timezone.toQTimeZone(), QTimeZone());

        station = KnowledgeDb::stationForIbnr(IBNR{8011160});
        QVERIFY(station.coordinate.isValid());
        QCOMPARE(station.timezone.toQTimeZone(), QTimeZone("Europe/Berlin"));
        QCOMPARE(station.country, CountryId{"DE"});

        station = KnowledgeDb::stationForIbnr(IBNR{8501687});
        QVERIFY(station.coordinate.isValid());
        QCOMPARE(station.timezone.toQTimeZone(), QTimeZone("Europe/Zurich"));
        QCOMPARE(station.country, CountryId{"CH"});

        // Aachen West, very close to the NL border, should be in DE timezone
        station = KnowledgeDb::stationForIbnr(IBNR{8000404});
        QVERIFY(station.coordinate.isValid());
        QCOMPARE(station.timezone.toQTimeZone(), QTimeZone("Europe/Berlin"));
        QCOMPARE(station.country, CountryId{"DE"});
    }

    void testUICLookup()
    {
        auto station = KnowledgeDb::stationForUic(UICStation{1234567});
        QVERIFY(!station.coordinate.isValid());
        QCOMPARE(station.timezone.toQTimeZone(), QTimeZone());

        station = KnowledgeDb::stationForUic({});
        QVERIFY(!station.coordinate.isValid());
        QCOMPARE(station.timezone.toQTimeZone(), QTimeZone());

        station = KnowledgeDb::stationForUic(UICStation{1001332});
        QVERIFY(station.coordinate.isValid());
        QCOMPARE(station.timezone.toQTimeZone(), QTimeZone("Europe/Helsinki"));
        QCOMPARE(station.country, CountryId{"FI"});
    }

    void testGaresConnexionsIdLookup()
    {
        auto station = KnowledgeDb::stationForGaresConnexionsId({});
        QVERIFY(!station.coordinate.isValid());
        QCOMPARE(station.timezone.toQTimeZone(), QTimeZone());

        station = KnowledgeDb::stationForGaresConnexionsId(GaresConnexionsId{"XXXXX"});
        QVERIFY(!station.coordinate.isValid());
        QCOMPARE(station.timezone.toQTimeZone(), QTimeZone());

        station = KnowledgeDb::stationForGaresConnexionsId(GaresConnexionsId{"FRAES"});
        QVERIFY(station.coordinate.isValid());
        QCOMPARE(station.timezone.toQTimeZone(), QTimeZone("Europe/Paris"));
        QCOMPARE(station.country, CountryId{"FR"});

        station = KnowledgeDb::stationForGaresConnexionsId(GaresConnexionsId{QStringLiteral("FRXYT")});
        QVERIFY(station.coordinate.isValid());
        QCOMPARE(station.timezone.toQTimeZone(), QTimeZone("Europe/Paris"));
        QCOMPARE(station.country, CountryId{"FR"});

        station = KnowledgeDb::stationForGaresConnexionsId(GaresConnexionsId{"CHGVA"});
        QEXPECT_FAIL("", "Wikidata does not supply ids for non-French stations yet", Continue);
        QVERIFY(station.coordinate.isValid());
        QEXPECT_FAIL("", "Wikidata does not supply ids for non-French stations yet", Continue);
        QCOMPARE(station.timezone.toQTimeZone(), QTimeZone("Europe/Zurich"));
        QEXPECT_FAIL("", "Wikidata does not supply ids for non-French stations yet", Continue);
        QCOMPARE(station.country, CountryId{"CH"});
    }

    void testCountryDb()
    {
        auto country = KnowledgeDb::countryForId(CountryId{});
        QCOMPARE(country.drivingSide, KnowledgeDb::DrivingSide::Unknown);
        QCOMPARE(country.powerPlugTypes, {Unknown});

        country = KnowledgeDb::countryForId(CountryId{"DE"});
        QCOMPARE(country.drivingSide, KnowledgeDb::DrivingSide::Right);
        QCOMPARE(country.powerPlugTypes, {TypeC|TypeF});
        country = KnowledgeDb::countryForId(CountryId{"GB"});
        QCOMPARE(country.drivingSide, KnowledgeDb::DrivingSide::Left);
        QCOMPARE(country.powerPlugTypes, {TypeG});
        country = KnowledgeDb::countryForId(CountryId{"GL"});
        QCOMPARE(country.drivingSide, KnowledgeDb::DrivingSide::Right);
    }

    void testPowerPlugCompat_data()
    {
        using namespace KnowledgeDb;

        QTest::addColumn<PowerPlugTypes>("plugs");
        QTest::addColumn<PowerPlugTypes>("sockets");
        QTest::addColumn<PowerPlugTypes>("failPlugs");
        QTest::addColumn<PowerPlugTypes>("failSockets");

        QTest::newRow("empty") << PowerPlugTypes{} << PowerPlugTypes{} << PowerPlugTypes{} << PowerPlugTypes{};
        QTest::newRow("DE-DE") << PowerPlugTypes{TypeC|TypeF} << PowerPlugTypes{TypeC|TypeF} << PowerPlugTypes{} << PowerPlugTypes{};
        QTest::newRow("DE-CH") << PowerPlugTypes{TypeC|TypeF} << PowerPlugTypes{TypeC|TypeJ} << PowerPlugTypes{TypeF} << PowerPlugTypes{TypeJ};
        QTest::newRow("CH-DE") << PowerPlugTypes{TypeC|TypeJ} << PowerPlugTypes{TypeC|TypeF} << PowerPlugTypes{TypeJ} << PowerPlugTypes{TypeF};
        QTest::newRow("DE-FR") << PowerPlugTypes{TypeC|TypeF} << PowerPlugTypes{TypeC|TypeE} << PowerPlugTypes{} << PowerPlugTypes{};
        QTest::newRow("DE-GB") << PowerPlugTypes{TypeC|TypeF} << PowerPlugTypes{TypeG} << PowerPlugTypes{TypeC|TypeF} << PowerPlugTypes{TypeG};
        QTest::newRow("DE-IT") << PowerPlugTypes{TypeC|TypeF} << PowerPlugTypes{TypeC|TypeF|TypeL} << PowerPlugTypes{} << PowerPlugTypes{TypeL};
        QTest::newRow("IT-DE") << PowerPlugTypes{TypeC|TypeF|TypeL} << PowerPlugTypes{TypeC|TypeF} << PowerPlugTypes{TypeL} << PowerPlugTypes{};
        QTest::newRow("DE-IL") << PowerPlugTypes{TypeC|TypeF} << PowerPlugTypes{TypeC|TypeH|TypeM} << PowerPlugTypes{TypeF} << PowerPlugTypes{TypeH|TypeM};
        QTest::newRow("DE-AO") << PowerPlugTypes{TypeC|TypeF} << PowerPlugTypes{TypeC} << PowerPlugTypes{TypeF} << PowerPlugTypes{};
        QTest::newRow("AO-DE") << PowerPlugTypes{TypeC} << PowerPlugTypes{TypeC|TypeF} << PowerPlugTypes{} << PowerPlugTypes{};
        QTest::newRow("DE-DK") << PowerPlugTypes{TypeC|TypeF} << PowerPlugTypes{TypeC|TypeE|TypeF|TypeK} << PowerPlugTypes{} << PowerPlugTypes{};
        QTest::newRow("DK-DE") << PowerPlugTypes{TypeC|TypeF|TypeE|TypeK} << PowerPlugTypes{TypeC|TypeF} << PowerPlugTypes{TypeK} << PowerPlugTypes{};
        QTest::newRow("DE-ZA") << PowerPlugTypes{TypeC|TypeF} << PowerPlugTypes{TypeC|TypeD|TypeM|TypeN} << PowerPlugTypes{TypeF} << PowerPlugTypes{TypeD|TypeM|TypeN};
        QTest::newRow("ZA-CH") << PowerPlugTypes{TypeC|TypeD|TypeM|TypeN} << PowerPlugTypes{TypeC|TypeJ} << PowerPlugTypes{TypeD|TypeM|TypeN} << PowerPlugTypes{TypeJ};
        QTest::newRow("ZA-DE") << PowerPlugTypes{TypeC|TypeD|TypeM|TypeN} << PowerPlugTypes{TypeC|TypeF} << PowerPlugTypes{TypeD|TypeM|TypeN} << PowerPlugTypes{TypeF};
        QTest::newRow("ZA-IT") << PowerPlugTypes{TypeC|TypeD|TypeM|TypeN} << PowerPlugTypes{TypeC|TypeF|TypeL} << PowerPlugTypes{TypeD|TypeM|TypeN} << PowerPlugTypes{TypeF|TypeL};
    }

    void testPowerPlugCompat()
    {
        using namespace KnowledgeDb;

        QFETCH(PowerPlugTypes, plugs);
        QFETCH(PowerPlugTypes, sockets);
        QFETCH(PowerPlugTypes, failPlugs);
        QFETCH(PowerPlugTypes, failSockets);

        QCOMPARE(KnowledgeDb::incompatiblePowerPlugs(plugs, sockets), failPlugs);
        QCOMPARE(KnowledgeDb::incompatiblePowerSockets(plugs, sockets), failSockets);
    }

    void testTimezoneForCountry()
    {
        using namespace KnowledgeDb;

        QCOMPARE(KnowledgeDb::timezoneForCountry(CountryId{"DE"}).toQTimeZone(), QTimeZone("Europe/Berlin"));
        QCOMPARE(KnowledgeDb::timezoneForCountry(CountryId{"FR"}).toQTimeZone(), QTimeZone("Europe/Paris"));
        QCOMPARE(KnowledgeDb::timezoneForCountry(CountryId{"BR"}).toQTimeZone(), QTimeZone());
    }

    void testUICCountryCodeLookup()
    {
        using namespace KnowledgeDb;

        QCOMPARE(KnowledgeDb::countryIdForUicCode(80), CountryId{"DE"});
        QCOMPARE(KnowledgeDb::countryIdForUicCode(0), CountryId{});
    }

    void testIso3Lookup()
    {
        using namespace KnowledgeDb;

        QCOMPARE(KnowledgeDb::countryIdFromIso3166_1alpha3(CountryId3{"ITA"}), CountryId{"IT"});
        QCOMPARE(KnowledgeDb::countryIdFromIso3166_1alpha3(CountryId3{"FOO"}), CountryId{});
    }
};

QTEST_APPLESS_MAIN(KnowledgeDbTest)

#include "knowledgedbtest.moc"
