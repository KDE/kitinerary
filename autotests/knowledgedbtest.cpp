/*
  SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KItinerary/CountryDb>

#include <config-kitinerary.h>
#include "knowledgedb/alphaid.h"
#include <knowledgedb/timezonedb.cpp>
#include "knowledgedb/trainstationdb.h"

#include <QDebug>
#include <QObject>
#include <QTest>
#include <QTimeZone>

using namespace Qt::Literals::StringLiterals;
using namespace KItinerary;
using namespace KItinerary::KnowledgeDb;

namespace KItinerary { namespace KnowledgeDb {
char *toString(CountryId c)
{
    using QTest::toString;
    return toString(c.toString());
}
}}

char *toString(const QTimeZone &tz)
{
    using QTest::toString;
    return toString(tz.id());
}

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

       QCOMPARE(id1.toString(), QLatin1StringView("ABC"));
       QCOMPARE(id2.toString(), QLatin1StringView("CBA"));

       constexpr ID3 id3;
       QVERIFY(!id3.isValid());
       QVERIFY(id3.toString().isEmpty());

       qDebug() << id1;
    }

    void testStationIdentifiers()
    {
        auto sncf = KnowledgeDb::SncfStationId(QStringLiteral("FRPNO"));
        QVERIFY(sncf.isValid());
        QCOMPARE(sncf.toString(), QLatin1StringView("FRPNO"));
        sncf = KnowledgeDb::SncfStationId(QStringLiteral("Abc"));
        QVERIFY(!sncf.isValid());
        sncf =  KnowledgeDb::SncfStationId(QStringLiteral("CHZID"));
        QVERIFY(sncf.isValid());
        QCOMPARE(sncf.toString(), QLatin1StringView("CHZID"));

        auto vrCode = KnowledgeDb::VRStationCode(QStringLiteral("HSL"));
        QVERIFY(vrCode.isValid());
        QCOMPARE(vrCode.toString(), QLatin1StringView("HSL"));
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

        // Berlin Gesundbrunnen has complex closing/reopening times in Wikidata that can confuse the generator
        station = KnowledgeDb::stationForIbnr(IBNR{8011102});
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

        // unassigned UIC code, but ambigiously used in DB tickets and SNCF API
        station = KnowledgeDb::stationForUic(UICStation{8003137});
        QVERIFY(!station.coordinate.isValid());
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
        QCOMPARE(country.powerPlugTypes, KnowledgeDb::PowerPlugTypes{TypeC|TypeF});
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

        QCOMPARE(timezoneForLocation(NAN, NAN, u"DE", {}), QTimeZone("Europe/Berlin"));
        QCOMPARE(timezoneForLocation(NAN, NAN, u"FR", {}), QTimeZone("Europe/Paris"));
        QCOMPARE(timezoneForLocation(NAN, NAN, u"BR", {}), QTimeZone());
        QCOMPARE(timezoneForLocation(NAN, NAN, u"US", {}), QTimeZone());
        QCOMPARE(timezoneForLocation(NAN, NAN, u"US", u"CA"), QTimeZone("America/Los_Angeles"));
    }

    void testTimezoneForLocation()
    {
        using namespace KnowledgeDb;

        // basic checks in all quadrants
        QCOMPARE(timezoneForLocation(52.4, 13.1, {}, {}), QTimeZone("Europe/Berlin"));
        QCOMPARE(timezoneForLocation(-8.0, -35.0, {}, {}), QTimeZone("America/Recife"));
        QCOMPARE(timezoneForLocation(-36.5, 175.0, {}, {}), QTimeZone("Pacific/Auckland"));
        QCOMPARE(timezoneForLocation(44.0, -79.5, {}, {}), QTimeZone("America/Toronto"));

        // Special case: Northern Vietnam has a Thai timezone
        QCOMPARE(timezoneForLocation(21.0, 106.0, {}, {}), QTimeZone("Asia/Bangkok"));

        // Maastricht (NL), very close to the BE border
        QCOMPARE(timezoneForLocation(50.8505, 5.6881, QString(), {}), QTimeZone("Europe/Amsterdam"));
        QCOMPARE(timezoneForLocation(50.8505, 5.6881, u"NL", {}), QTimeZone("Europe/Amsterdam"));

        // Aachen, at the BE/DE/NL corner
        QCOMPARE(timezoneForLocation(50.7717, 6.04235, QString(), {}), QTimeZone("Europe/Berlin"));
        QCOMPARE(timezoneForLocation(50.7717, 6.04235, u"DE", {}), QTimeZone("Europe/Berlin"));
        //QCOMPARE(timezoneForLocation(50.7727, 6.01565, QString()), QTimeZone("Europe/Brussels"));
        QCOMPARE(timezoneForLocation(50.7727, 6.01565, u"BE", {}), QTimeZone("Europe/Brussels"));

        // Geneva (CH), very close to the FR border
        QCOMPARE(timezoneForLocation(46.23213, 6.10636, u"CH", {}), QTimeZone("Europe/Zurich"));

        // Busingen (DE), enclosed by CH, and in theory its own timezone (which we ignore)
        QCOMPARE(timezoneForLocation(47.69947, 8.68833, u"DE", {}), QTimeZone("Europe/Berlin"));
        QCOMPARE(timezoneForLocation(47.67904, 8.68813, {}, {}), QTimeZone("Europe/Zurich"));

        // Baarle, the ultimate special case, NL/BE differs house by house
        QCOMPARE(timezoneForLocation(51.44344, 4.93373, u"BE", {}), QTimeZone("Europe/Brussels"));
        QCOMPARE(timezoneForLocation(51.44344, 4.93373, u"NL", {}), QTimeZone("Europe/Amsterdam"));
        const auto tz = timezoneForLocation(51.44344, 4.93373, {}, {});
        QVERIFY(tz == QTimeZone("Europe/Amsterdam") || tz == QTimeZone("Europe/Brussels"));

        // Eliat Airport (IL), close to JO, and with a minor timezone variation due to different weekends
        QCOMPARE(timezoneForLocation(29.72530, 35.00598, u"IL", {}), QTimeZone("Asia/Jerusalem"));
        QCOMPARE(timezoneForLocation(29.60908, 35.02038, u"JO", {}), QTimeZone("Asia/Amman"));

        // Tijuana (MX), close to US, tests equivalent tz search in the neighbouring country
        QCOMPARE(timezoneForLocation(32.54274, -116.97505, u"MX", {}), QTimeZone("America/Tijuana"));
        QCOMPARE(timezoneForLocation(32.55783, -117.04773, u"US", {}), QTimeZone("America/Los_Angeles"));

        // Cordoba (AR), AR has several sub-zones that are all equivalent
        QCOMPARE(timezoneForLocation(-31.4, -64.2, u"AR", {}), QTimeZone("America/Argentina/Buenos_Aires"));

        // polar regions
        QCOMPARE(timezoneForLocation(-90.0, 0.0, {}, {}), QTimeZone());
        QCOMPARE(timezoneForLocation(90.0, 0.0, {}, {}), QTimeZone());

        // Hong Kong seems problematic on FreeBSD
        QCOMPARE(timezoneForLocation(22.31600, 113.93688, {}, {}), QTimeZone("Asia/Hong_Kong"));

        // coordinates not provided
        QCOMPARE(timezoneForLocation(NAN, NAN, u"LU", {}), QTimeZone("Europe/Luxembourg"));
    }

    void testUICCountryCodeLookup()
    {
        using namespace KnowledgeDb;

        QCOMPARE(KnowledgeDb::countryIdForUicCode(80), CountryId{"DE"});
        QCOMPARE(KnowledgeDb::countryIdForUicCode(0), CountryId{});
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

    void testFinishStationCode()
    {
        constexpr const VRStationCode hki("HKI\0");
        QVERIFY(hki.value() > 0);
        auto code = VRStationCode(u"HKI"_s);
        QVERIFY(hki == code);
        QCOMPARE(hki.toString(), "HKI"_L1);
        QCOMPARE(code.toString(), "HKI"_L1);

        constexpr const VRStationCode ol("OL\0\0");
        QVERIFY(ol.isValid());
        QVERIFY(ol != hki);
        QCOMPARE(ol.toString(), "OL"_L1);

        constexpr const VRStationCode kja("KJ[\0");
        QVERIFY(kja.isValid());
        code = VRStationCode(u"KJÄ"_s);
        QVERIFY(code.isValid());
        QVERIFY(kja == code);
        QCOMPARE(kja.toString(), u"KJÄ");
        QCOMPARE(code.toString(), u"KJÄ");
    }

    void testFinishStationCodeLookup()
    {
        auto station = KnowledgeDb::stationForVRStationCode(VRStationCode(QStringLiteral("HKI")));
        QVERIFY(station.coordinate.isValid());

        station = KnowledgeDb::stationForVRStationCode(VRStationCode(u"KJÄ"_s));
        QVERIFY(station.coordinate.isValid());

        station = KnowledgeDb::stationForVRStationCode(VRStationCode(QStringLiteral("BLÄ")));
        QVERIFY(!station.coordinate.isValid());
    }

    void testIsPlausibleTimeZone_data()
    {
        QTest::addColumn<QString>("tzId");
        QTest::addColumn<float>("lat");
        QTest::addColumn<float>("lon");
        QTest::addColumn<QString>("country");
        QTest::addColumn<QString>("region");
        QTest::addColumn<bool>("result");

        QTest::newRow("no-location") << "Europe/Berlin" << NAN << NAN << QString() << QString() << true;
        QTest::newRow("coord-match") << "Europe/Berlin" << 52.0f << 13.0f << QString() << QString() << true;
        QTest::newRow("country-match") << "Europe/Berlin" << NAN << NAN << "DE" << QString() << true;
        QTest::newRow("country-ambiguous") << "America/Los_Angeles" << NAN << NAN << "US" << QString() << true;
        QTest::newRow("region-match") << "America/Los_Angeles" << NAN << NAN << "US" << "CA" << true;
        QTest::newRow("region-mismatch") << "America/Los_Angeles" << NAN << NAN << "US" << "FL" << false;
        QTest::newRow("coutry-mismatch") << "Europe/Berlin" << NAN << NAN << "PT" << QString() << false;
        QTest::newRow("coord-mismatch") << "America/Los_Angeles" << 52.0f << 13.0f << QString() << QString() << false;
        QTest::newRow("coord-equivalent") << "Europe/Paris" << 52.0f << 13.0f << QString() << QString() << true;
        QTest::newRow("country-equivalent") << "Europe/Paris" << NAN << NAN << "FR" << QString() << true;
        QTest::newRow("region-equivalent") << "America/Tijuana" << NAN << NAN << "US" << "CA" << true;
    }

    void testIsPlausibleTimeZone()
    {
        QFETCH(QString, tzId);
        QTimeZone tz(tzId.toUtf8());
        QVERIFY(tz.isValid());
        QFETCH(float, lat);
        QFETCH(float, lon);
        QFETCH(QString, country);
        QFETCH(QString, region);
        QFETCH(bool, result);

        QCOMPARE(KnowledgeDb::isPlausibleTimeZone(tz, lat, lon, country, region), result);
    }
};

QTEST_APPLESS_MAIN(KnowledgeDbTest)

#include "knowledgedbtest.moc"
