/*
  SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
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

namespace KItinerary { namespace KnowledgeDb {
char *toString(Tz tz)
{
    using QTest::toString;
    return toString(tzId(tz));
}
char *toString(CountryId c)
{
    using QTest::toString;
    return toString(c.toString());
}
}}

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

    void testStationIdentifiers()
    {
        auto sncf = KnowledgeDb::SncfStationId(QStringLiteral("FRPNO"));
        QVERIFY(sncf.isValid());
        QCOMPARE(sncf.toString(), QLatin1String("FRPNO"));
        sncf = KnowledgeDb::SncfStationId(QStringLiteral("Abc"));
        QVERIFY(!sncf.isValid());
        sncf =  KnowledgeDb::SncfStationId(QStringLiteral("CHZID"));
        QVERIFY(sncf.isValid());
        QCOMPARE(sncf.toString(), QLatin1String("CHZID"));

        auto vrCode = KnowledgeDb::VRStationCode(QStringLiteral("HSL"));
        QVERIFY(vrCode.isValid());
        QCOMPARE(vrCode.toString(), QLatin1String("HSL"));
    }

    void testIBNRLookup()
    {
        auto station = KnowledgeDb::stationForIbnr(IBNR{1234567});
        QVERIFY(!station.coordinate.isValid());

        station = KnowledgeDb::stationForIbnr({});
        QVERIFY(!station.coordinate.isValid());

        station = KnowledgeDb::stationForIbnr(IBNR{8011160});
        QVERIFY(station.coordinate.isValid());
        QCOMPARE(station.country, CountryId{"DE"});

        station = KnowledgeDb::stationForIbnr(IBNR{8501687});
        QVERIFY(station.coordinate.isValid());
        QCOMPARE(station.country, CountryId{"CH"});

        // Aachen West, very close to the NL border, should be in DE timezone
        station = KnowledgeDb::stationForIbnr(IBNR{8000404});
        QVERIFY(station.coordinate.isValid());
        QCOMPARE(station.country, CountryId{"DE"});
    }

    void testUICLookup()
    {
        auto station = KnowledgeDb::stationForUic(UICStation{1234567});
        QVERIFY(!station.coordinate.isValid());

        station = KnowledgeDb::stationForUic({});
        QVERIFY(!station.coordinate.isValid());

        station = KnowledgeDb::stationForUic(UICStation{1001332});
        QVERIFY(station.coordinate.isValid());
        QCOMPARE(station.country, CountryId{"FI"});
    }

    void testSncfStationIdLookup()
    {
        auto station = KnowledgeDb::stationForSncfStationId({});
        QVERIFY(!station.coordinate.isValid());

        station = KnowledgeDb::stationForSncfStationId(SncfStationId{"XXXXX"});
        QVERIFY(!station.coordinate.isValid());

        station = KnowledgeDb::stationForSncfStationId(SncfStationId{"FRAES"});
        QVERIFY(station.coordinate.isValid());
        QCOMPARE(station.country, CountryId{"FR"});

        station = KnowledgeDb::stationForSncfStationId(SncfStationId{QStringLiteral("FRXYT")});
        QVERIFY(station.coordinate.isValid());
        QCOMPARE(station.country, CountryId{"FR"});

        station = KnowledgeDb::stationForSncfStationId(SncfStationId{"CHGVA"});
        QVERIFY(station.coordinate.isValid());
        QCOMPARE(station.country, CountryId{"CH"});
        station = KnowledgeDb::stationForSncfStationId(SncfStationId{"FRHWO"}); // alias for CHGVA...
        QVERIFY(station.coordinate.isValid());
        QCOMPARE(station.country, CountryId{"CH"});

        station = KnowledgeDb::stationForSncfStationId(SncfStationId{"NLAMA"}); // vs. SNCB ID of NLASC
        QVERIFY(station.coordinate.isValid());
        QCOMPARE(station.country, CountryId{"NL"});
    }

    void testBenerailStationIdLookup()
    {
        auto station = KnowledgeDb::stationForBenerailId({});
        QVERIFY(!station.coordinate.isValid());

        station = KnowledgeDb::stationForBenerailId(BenerailStationId{"XXXXX"});
        QVERIFY(!station.coordinate.isValid());

        station = KnowledgeDb::stationForBenerailId(BenerailStationId{"NLASC"});
        QVERIFY(station.coordinate.isValid());
        QCOMPARE(station.country, CountryId{"NL"});
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

        QCOMPARE(toQTimeZone(timezoneForCountry(CountryId{"DE"})), QTimeZone("Europe/Berlin"));
        QCOMPARE(toQTimeZone(timezoneForCountry(CountryId{"FR"})), QTimeZone("Europe/Paris"));
        QCOMPARE(toQTimeZone(timezoneForCountry(CountryId{"BR"})), QTimeZone());
    }

    void testCountryForTimezone()
    {
        using namespace KnowledgeDb;

        QCOMPARE(countryForTimezone(Tz::Europe_Busingen), CountryId{"DE"});
        QCOMPARE(countryForTimezone(Tz::America_Los_Angeles), CountryId{"US"});
        QCOMPARE(countryForTimezone(Tz::Asia_Kuching), CountryId{"MY"});
        QCOMPARE(countryForTimezone(Tz::Asia_Bangkok), CountryId{});
        QCOMPARE(countryForTimezone(Tz::Asia_Ho_Chi_Minh), CountryId{"VN"});
    }

    void testTimezoneForLocation()
    {
        using namespace KnowledgeDb;

        // basic checks in all quadrants
        QCOMPARE(timezoneForCoordinate(52.4, 13.1), Tz::Europe_Berlin);
        QCOMPARE(timezoneForCoordinate(-8.0, -35.0), Tz::America_Recife);
        QCOMPARE(timezoneForCoordinate(-36.5, 175.0), Tz::Pacific_Auckland);
        QCOMPARE(timezoneForCoordinate(44.0, -79.5), Tz::America_Toronto);

        // Special case: Northern Vietnam has a Thai timezone
        QCOMPARE(timezoneForCoordinate(21.0, 106.0), Tz::Asia_Bangkok);

        // Maastricht (NL), very close to the BE border
        QCOMPARE(timezoneForLocation(50.8505, 5.6881, CountryId{}), Tz::Europe_Amsterdam);
        QCOMPARE(timezoneForLocation(50.8505, 5.6881, CountryId{"NL"}), Tz::Europe_Amsterdam);

        // Aachen, at the BE/DE/NL corner
        QCOMPARE(timezoneForLocation(50.7717, 6.04235, CountryId{}), Tz::Europe_Berlin);
        QCOMPARE(timezoneForLocation(50.7717, 6.04235, CountryId{"DE"}), Tz::Europe_Berlin);
        //QCOMPARE(timezoneForLocation(50.7727, 6.01565, CountryId{}), Tz::Europe_Brussels);
        QCOMPARE(timezoneForLocation(50.7727, 6.01565, CountryId{"BE"}), Tz::Europe_Brussels);

        // Geneva (CH), very close to the FR border
        QCOMPARE(timezoneForLocation(46.23213, 6.10636, CountryId{"CH"}), Tz::Europe_Zurich);

        // Busingen (DE), enclosed by CH, and in theory its own timezone (which we ignore)
        QCOMPARE(timezoneForLocation(47.69947, 8.68833, CountryId{"DE"}), Tz::Europe_Berlin);
        QCOMPARE(timezoneForLocation(47.67904, 8.68813, {}), Tz::Europe_Zurich);

        // Baarle, the ultimate special case, NL/BE differs house by house
        QCOMPARE(timezoneForLocation(51.44344, 4.93373, CountryId{"BE"}), Tz::Europe_Brussels);
        QCOMPARE(timezoneForLocation(51.44344, 4.93373, CountryId{"NL"}), Tz::Europe_Amsterdam);
        bool ambiguous = false;
        auto tz = timezoneForCoordinate(51.44344, 4.93373, &ambiguous);
        QVERIFY(ambiguous);
        QVERIFY(tz == Tz::Europe_Amsterdam || tz == Tz::Europe_Brussels);

        // Eliat Airport (IL), close to JO, and with a minor timezone variation due to different weekends
        QCOMPARE(timezoneForLocation(29.72530, 35.00598, CountryId{"IL"}), Tz::Asia_Jerusalem);
        QCOMPARE(timezoneForLocation(29.60908, 35.02038, CountryId{"JO"}), Tz::Asia_Amman);

        // Tijuana (MX), close to US, tests equivalent tz search in the neighbouring country
        QCOMPARE(timezoneForLocation(32.54274, -116.97505, CountryId{"MX"}), Tz::America_Tijuana);
        QCOMPARE(timezoneForLocation(32.55783, -117.04773, CountryId{"US"}), Tz::America_Los_Angeles);

        // Cordoba (AR), AR has several sub-zones that are all equivalent
        QCOMPARE(timezoneForLocation(-31.4, -64.2, CountryId{"AR"}), Tz::America_Argentina_Cordoba);

        // polar regions
        QCOMPARE(timezoneForCoordinate(-90.0, 0.0), Tz::Undefined);
        QCOMPARE(timezoneForCoordinate(90.0, 0.0), Tz::Undefined);

        // Hong Kong seems problematic on FreeBSD
        QCOMPARE(timezoneForCoordinate(22.31600, 113.93688), Tz::Asia_Hong_Kong);
    }

    void testCountryFromCoordinate()
    {
        using namespace KnowledgeDb;

        // basic tests
        QCOMPARE(countryForCoordinate(52.4, 13.1), CountryId{"DE"});
        QCOMPARE(countryForCoordinate(-8.0, -35.0), CountryId{"BR"});
        QCOMPARE(countryForCoordinate(-36.5, 175.0), CountryId{"NZ"});
        QCOMPARE(countryForCoordinate(44.0, -79.5), CountryId{"CA"});

        // ambiguous locations
        QCOMPARE(countryForCoordinate(51.44344, 4.93373), CountryId{});

        // special case: northern Vietnam has a non-VN timezone (not the case anywhere else in the world up to 2020a)
        QCOMPARE(countryForCoordinate(21.0, 106.0), CountryId{});
        QCOMPARE(countryForCoordinate(10.5, 107.0), CountryId{"VN"});
        QCOMPARE(countryForCoordinate(13.7, 100.4), CountryId{});

        // disputed areas
        QCOMPARE(countryForCoordinate(45.0, 34.0), CountryId{});

        // overseas territories with separate ISO 3166-1 codes
        QCOMPARE(countryForCoordinate(4.8, -52.3), CountryId{"GF"}); // could also be "FR"
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

    void testIndianRailwaysStationCodeLookup()
    {
        auto station = KnowledgeDb::stationForIndianRailwaysStationCode(QString());
        QVERIFY(!station.coordinate.isValid());

        station = KnowledgeDb::stationForIndianRailwaysStationCode(QStringLiteral("NDLS"));
        QVERIFY(station.coordinate.isValid());
        QCOMPARE(station.country, CountryId{"IN"});

        station = KnowledgeDb::stationForIndianRailwaysStationCode(QStringLiteral("ndls"));
        QVERIFY(!station.coordinate.isValid());
    }

    void testFinishStationCodeLookup()
    {
        auto station = KnowledgeDb::stationForVRStationCode(VRStationCode(QStringLiteral("HKI")));
        QVERIFY(station.coordinate.isValid());

        station = KnowledgeDb::stationForVRStationCode(VRStationCode(QStringLiteral("BLÄ")));
        QVERIFY(!station.coordinate.isValid());
    }
};

QTEST_APPLESS_MAIN(KnowledgeDbTest)

#include "knowledgedbtest.moc"
