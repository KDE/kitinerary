/*
  SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <config-kitinerary.h>
#include "knowledgedb/airportdb.h"
#include <knowledgedb/timezonedb.cpp>

#include <KItinerary/LocationUtil>

#include <QDebug>
#include <QObject>
#include <QTest>
#include <QTimeZone>

#include <cmath>

Q_DECLARE_METATYPE(KItinerary::KnowledgeDb::IataCode)

using namespace KItinerary;
using namespace KItinerary::KnowledgeDb;

#define s(x) QStringLiteral(x)

namespace KItinerary { namespace KnowledgeDb {
char *toString(const IataCode &code)
{
    using QTest::toString;
    return toString(code.toString());
}
char *toString(const CountryId &code)
{
    using QTest::toString;
    return toString(code.toString());
}
}}

char *toString(const QTimeZone &tz)
{
    using QTest::toString;
    return toString(tz.id());
}

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

        // HKG seems to cause trouble on FreeBSD
        const auto coord = KnowledgeDb::coordinateForAirport(KnowledgeDb::IataCode{"HKG"});
        QVERIFY(coord.isValid());
        QVERIFY(LocationUtil::distance(coord.latitude, coord.longitude, 22.31600, 113.93688) < 500);
        QCOMPARE(KnowledgeDb::timezoneForLocation(coord.latitude, coord.longitude, {}, {}), QTimeZone("Asia/Hong_Kong"));
        const auto country = KnowledgeDb::countryForAirport(KnowledgeDb::IataCode{"HKG"});
        QCOMPARE(country, KnowledgeDb::CountryId{"CN"});
        QCOMPARE(KnowledgeDb::timezoneForLocation(coord.latitude, coord.longitude, u"CN", {}), QTimeZone("Asia/Shanghai"));
        tz = KnowledgeDb::timezoneForAirport(KnowledgeDb::IataCode{"HKG"});
        QCOMPARE(tz.id(), QByteArray("Asia/Shanghai"));
    }

    void iataLookupTest_data()
    {
        QTest::addColumn<QString>("name");
        QTest::addColumn<KnowledgeDb::IataCode>("iataCode");

        // via unique fragment lookup
        QTest::newRow("TXL1") << s("Flughafen Berlin-Tegel") << KnowledgeDb::IataCode{"TXL"};
        QTest::newRow("TXL2") << s("TEGEL") << KnowledgeDb::IataCode{"TXL"};
        QTest::newRow("CDG") << s("Paris Charles de Gaulle") << KnowledgeDb::IataCode{"CDG"};
        QTest::newRow("ZRH") << s("Zürich") << KnowledgeDb::IataCode{"ZRH"};
        QTest::newRow("AMS") << s("AMSTERDAM, NL (SCHIPHOL AIRPORT)") << KnowledgeDb::IataCode{"AMS"};
        QTest::newRow("LHR") << s("London Heathrow") << KnowledgeDb::IataCode{"LHR"};

        // via non-unique fragment lookup
        QTest::newRow("JFK") << s("John F. Kennedy International Airport") << KnowledgeDb::IataCode{"JFK"};
        QTest::newRow("SFO") << s("San Francisco International") << KnowledgeDb::IataCode{"SFO"};
        QTest::newRow("DUS") << s("Düsseldorf International") << KnowledgeDb::IataCode{"DUS"};
        QTest::newRow("LCY") << s("London City") << KnowledgeDb::IataCode{"LCY"};
        QTest::newRow("DTW") << s("DETROIT, MI (METROPOLITAN WAYNE CO)") << KnowledgeDb::IataCode{"DTW"};

        // string normalization
        QTest::newRow("GRU1") << s("Sao Paulo-Guarulhos International") << KnowledgeDb::IataCode{"GRU"};
        QTest::newRow("GRU2") << s("São Paulo-Guarulhos International") << KnowledgeDb::IataCode{"GRU"};
        QTest::newRow("ZRH2") << s("Zurich") << KnowledgeDb::IataCode{"ZRH"};
        QTest::newRow("DUS2") << s("Dusseldorf International") << KnowledgeDb::IataCode{"DUS"};
        QTest::newRow("LEI1") << s("Almeria") << KnowledgeDb::IataCode{"LEI"};
        QTest::newRow("LEI2") << s("ALMERÍA") << KnowledgeDb::IataCode{"LEI"};
        QTest::newRow("KEF1") << s("Keflavík") << KnowledgeDb::IataCode{"KEF"};
        QTest::newRow("KEF2") << s("Keflavik") << KnowledgeDb::IataCode{"KEF"};

        // alternative transliterations
        QTest::newRow("DUS3") << s("Duesseldorf International") << KnowledgeDb::IataCode{"DUS"};
        QTest::newRow("ZRH3") << s("Zuerich") << KnowledgeDb::IataCode{"ZRH"};

        // IATA code contained in name
        QTest::newRow("FRA") << s("Frankfurt FRA") << KnowledgeDb::IataCode{"FRA"};

        // multiple unique hits / unique hit on valid (but wrong) IATA code
        QTest::newRow("GMP") << s("GIMPO INTERNATIONAL TERMINAL I - SKY CITY INTERNATIONAL TERMINAL") << KnowledgeDb::IataCode{"GMP"};

        // Amadeus/BCD airport names containing city/country data too, and using "INTL" abbreviation
        QTest::newRow("SFO2") << s("SAN FRANCISCO CA SAN FRANCISCO INTL") << KnowledgeDb::IataCode{"SFO"};
        QTest::newRow("SEA") << s("SEATTLE US - SEATTLE TACOMA INTL") << KnowledgeDb::IataCode{"SEA"};
    }

    void iataLookupTest()
    {
        QFETCH(QString, name);
        QFETCH(KnowledgeDb::IataCode, iataCode);

        QCOMPARE(KnowledgeDb::iataCodesFromName(name), (std::vector<IataCode>{iataCode}));
    }

    void iataCodeMultiLookupTest()
    {
        // duplicate unique fragments
        QCOMPARE(KnowledgeDb::iataCodesFromName(QStringLiteral("OSAKA JP KANSAI INTERNATIONAL")), (std::vector<IataCode>{IataCode{"ITM"}, IataCode{"KIX"}}));

        // insufficient non-unique fragments
        QCOMPARE(KnowledgeDb::iataCodesFromName(QStringLiteral("Stuttgart")), (std::vector<IataCode>{IataCode{"SGT"}, IataCode{"STR"}}));
        QCOMPARE(KnowledgeDb::iataCodesFromName(QStringLiteral("Frankfurt")), (std::vector<IataCode>{IataCode{"FRA"}, IataCode{"HHN"}}));
        QCOMPARE(KnowledgeDb::iataCodesFromName(QStringLiteral("Brussels")), (std::vector<IataCode>{IataCode{"BRU"}, IataCode{"CRL"}}));

        // multiple unique hits / unique hit on valid (but wrong) IATA code
        QCOMPARE(KnowledgeDb::iataCodesFromName(QStringLiteral("SEOUL KR GIMPO INTERNATIONAL TERMINAL I - SKY CITY INTERNATIONAL TERMINAL")), (std::vector<IataCode>{IataCode{"GMP"}, IataCode{"ICN"}}));

        // "wrong" use of "international"
        QCOMPARE(KnowledgeDb::iataCodesFromName(QStringLiteral("FRANKFURT DE - FRANKFURT INTL")), (std::vector<IataCode>{IataCode{"FRA"}, IataCode{"HHN"}}));

        // not unique or conflicting
        QCOMPARE(KnowledgeDb::iataCodesFromName(QStringLiteral("Flughafen Berlin")), (std::vector<IataCode>{IataCode{"BER"}, IataCode{"BML"}, IataCode{"TXL"}}));
        QCOMPARE(KnowledgeDb::iataCodesFromName(QStringLiteral("Charles de Gaulle Orly")), (std::vector<IataCode>{IataCode{"CDG"}, IataCode{"ORY"}}));
        QCOMPARE(KnowledgeDb::iataCodesFromName(QStringLiteral("Brussels Airport, BE")), (std::vector<IataCode>{IataCode{"BRU"}, IataCode{"CRL"}}));
        QCOMPARE(KnowledgeDb::iataCodesFromName(QStringLiteral("BEIJING CN CAPITAL INTL")), (std::vector<IataCode>{IataCode{"PEK"}, IataCode{"PKX"}}));
    }

    void countryDataTest()
    {
        auto iso = KnowledgeDb::countryForAirport(KnowledgeDb::IataCode{});
        QVERIFY(!iso.isValid());

        iso = KnowledgeDb::countryForAirport(KnowledgeDb::IataCode{"TXL"});
        QCOMPARE(iso, KnowledgeDb::CountryId{"DE"});
    }

    void airportLocationTest_data()
    {
        QTest::addColumn<QString>("iata");
        QTest::addColumn<float>("lat");
        QTest::addColumn<float>("lon");
        QTest::addColumn<int>("dist");

        // "single" entry airports
        QTest::newRow("AGP") << s("AGP") << 36.67608f << -4.49095f << 100;
        QTest::newRow("AMS") << s("AMS") << 52.3095230f << 4.7621813f << 50;
        QTest::newRow("ARN") << s("ARN") << 59.64927f << 17.92956f << 50;
        QTest::newRow("BER") << s("BER") << 52.36444f << 13.50964f << 150;
        QTest::newRow("BLR") << s("BLR") << 13.20023f << 77.70972f << 150;
        QTest::newRow("BRE") << s("BRE") << 53.05266f << 8.78692f << 50;
        QTest::newRow("BRU") << s("BRU") << 50.8985255f << 4.4830282f << 50;
        QTest::newRow("BUD") << s("BUD") << 47.43279f << 19.26115f << 100;
        QTest::newRow("CGN") << s("CGN") << 50.87856f << 7.12107f << 150;
        QTest::newRow("CPH") << s("CPH") << 55.6295693f << 12.6492994f << 50;
        QTest::newRow("DEL") << s("DEL") << 28.55681f << 77.08718f << 50;
        QTest::newRow("DEN") << s("DEN") << 39.84790f << -104.67340f << 150;
        QTest::newRow("DOH") << s("DOH") << 25.25854f << 51.61507f << 400; // ok-ish, w212459176 interfering, n7052290435 out of range
        QTest::newRow("DUB") << s("DUB") << 53.4273328f << -6.2437352f << 150;
        QTest::newRow("DUS") << s("DUS") << 51.27889f << 6.76566f << 150;
        QTest::newRow("EAP") << s("EAP") << 47.59960f << 7.53144f << 150;
        QTest::newRow("EDI") << s("EDI") << 55.9483110f << -3.36353370f << 250;
        QTest::newRow("EWR") << s("EWR") << 40.69049f << -74.17765f << 250;
        QTest::newRow("FCO") << s("FCO") << 41.79348f << 12.25208f << 50;
        QTest::newRow("FRA") << s("FRA") << 50.05100f << 8.571590f << 50;
        QTest::newRow("GDN") << s("GDN") << 54.38234f << 18.46640f << 150;
        QTest::newRow("GLA") << s("GLA") << 55.86405f << -4.43181f << 50;
        QTest::newRow("GOT") << s("GOT") << 57.66771f << 12.29549f << 150;
        QTest::newRow("GRU") << s("GRU") << -23.42560f << -46.48165f << 200;
        QTest::newRow("GVA") << s("GVA") << 46.23020f << 6.10828f << 200;
        QTest::newRow("HAJ") << s("HAJ") << 52.45849f << 9.69898f << 50;
        QTest::newRow("HAM") << s("HAM") << 53.63214f << 10.00648f << 100;
        QTest::newRow("HEL") << s("HEL") << 60.31619f << 24.96914f << 100;
        QTest::newRow("HFS") << s("HFS") << 60.02591f << 13.58202f << 50;
        QTest::newRow("HKG") << s("HKG") << 22.31569f << 113.93605f << 100;
        QTest::newRow("KEF") << s("KEF") << 63.99663f << -22.62355f << 200;
        QTest::newRow("LAX") << s("LAX") << 33.94356f << -118.40786f << 150;
        QTest::newRow("LEI") << s("LEI") << 36.84775f << -2.37242f << 50;
        QTest::newRow("LEJ") << s("LEJ") << 51.42020f << 12.22122f << 400; // we get the station here, which is fine
        QTest::newRow("LIS") << s("LIS") << 38.76876f << -9.12844f << 50;
        QTest::newRow("LUX") << s("LUX") << 49.63506f << 6.21650f << 200;
        QTest::newRow("LYS") << s("LYS") << 45.72065f << 5.07807f << 150;
        QTest::newRow("MUC") << s("MUC") << 48.35378f << 11.78633f << 50;
        QTest::newRow("NRT") << s("NRT") << 35.76462f << 140.38615f << 100; // technically a multi-terminal airport, but T1 is reasonable as all ways there pass T2
        QTest::newRow("NUE") << s("NUE") << 49.49411f << 11.07867f << 50;
        QTest::newRow("ORD") << s("ORD") << 41.97779f << -87.90269f << 50;
        QTest::newRow("OSL") << s("OSL") << 60.19361f << 11.09758f << 100;
        QTest::newRow("OTP") << s("OTP") << 44.57040f << 26.07763f << 200;
        QTest::newRow("OUL") << s("OUL") << 64.92865f << 25.37406f << 50;
        QTest::newRow("PDX") << s("PDX") << 45.58833f << -122.59240f << 150;
        QTest::newRow("PRG") << s("PRG") << 50.10640f << 14.26784f << 100;
        QTest::newRow("PVG") << s("PVG") << 31.15240f << 121.80214f << 100;
        QTest::newRow("REC") << s("REC") << -8.1314735f << -34.9177565f << 150;
        QTest::newRow("RIG") << s("RIX") << 56.92188f << 23.97976f << 50;
        QTest::newRow("SFO") << s("SFO") << 37.6162238f << -122.3915235f << 50;
        QTest::newRow("SHA") << s("SHA") << 31.19624f << 121.32377f << 200;
        QTest::newRow("STR") << s("STR") << 48.69052f << 9.19302f << 50;
        QTest::newRow("SXB") << s("SXB") << 48.54444f << 7.62783f << 50;
        QTest::newRow("TLL") << s("TLL") << 59.41685f << 24.79899f << 150;
        QTest::newRow("TLS") << s("TLS") << 43.63146f << 1.37364f << 100;
        QTest::newRow("TPE") << s("TPE") << 25.07719f <<  121.23250f << 350; // still ok-ish
        QTest::newRow("TXL") << s("TXL") << 52.55392f << 13.29208f << 100;
        QTest::newRow("VIE") << s("VIE") << 48.12024f << 16.56431f << 50;
        QTest::newRow("YOW") << s("YOW") << 45.32277f << -75.66726f << 100;
        QTest::newRow("ZRH") << s("ZRH") << 47.45024f << 8.56207f << 50;

        // "multi" (entry) airports, ie. those basically consisting of multiple separate airports in close proximity
        // for those all we can do here is verifyingh we get a stable result for *any* of its terminals,
        // for actually selecting the right terminal we'd need more detailed API
        QTest::newRow("BCN") << s("BCN") << 41.28891f << 2.07385f << 100; // T1
        QTest::newRow("BOM") << s("BOM") << 19.09929f << 72.87440f << 100; // T2
        QTest::newRow("CDG") << s("CDG") << 49.00397f << 2.57107f << 100; // T2
        QTest::newRow("ICN") << s("ICN") << 37.44750f << 126.45243f << 100; // T1
        QTest::newRow("LHR") << s("LHR") << 51.47207f << -0.48812f << 100; // T5
        QTest::newRow("MAD") << s("MAD") << 40.46791f << -3.57119f << 50; // T1-3
        //QTest::newRow("MAD") << s("MAD") << 40.49127f << -3.59243f << 100; // T4
        QTest::newRow("MXP") << s("MXP") << 45.64825f << 8.72300f << 100; // T2
        QTest::newRow("PEK") << s("PEK") << 40.05252f << 116.60973f << 100; // T3
    }

    void airportLocationTest()
    {
        QFETCH(QString, iata);
        QFETCH(float, lat);
        QFETCH(float, lon);
        QFETCH(int, dist);

        const auto coord = KnowledgeDb::coordinateForAirport(KnowledgeDb::IataCode{iata});
        QVERIFY(coord.isValid());

        const auto d = LocationUtil::distance(coord.latitude, coord.longitude, lat, lon);
        qDebug() << coord.latitude << coord.longitude << d << (dist - d);

        QEXPECT_FAIL("BUD", "closed terminal 1 (w8557242) interfering", Continue);
        QEXPECT_FAIL("GLA", "airport is not a polygon in OSM", Continue);
        QEXPECT_FAIL("PRG", "private/military terminals 3 and 4 interfering", Continue);
        QEXPECT_FAIL("PVG", "complicated", Continue);
        QEXPECT_FAIL("RIG", "open polygon in OSM", Continue);
        QEXPECT_FAIL("SXF", "w630509626 (government terminal) interfering", Continue);

        QEXPECT_FAIL("BCN", "complicated", Continue);
        QEXPECT_FAIL("BOM", "complicated", Continue);
        QEXPECT_FAIL("CDG", "complicated", Continue);
        QEXPECT_FAIL("ICN", "complicated", Continue);
        QEXPECT_FAIL("LHR", "complicated", Continue);
        QEXPECT_FAIL("MXP", "complicated", Continue);
        QEXPECT_FAIL("PEK", "complicated", Continue);

        QVERIFY(d <= dist);
    }
};

QTEST_APPLESS_MAIN(AirportDbTest)

#include "airportdbtest.moc"
