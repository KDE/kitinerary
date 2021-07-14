/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KItinerary/LocationUtil>
#include <KItinerary/Place>

#include <QDebug>
#include <QObject>
#include <QTest>

#define _(x) QStringLiteral(x)

using namespace KItinerary;

class LocationUtilTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testLocationCompare()
    {
        Airport txlCoord;
        txlCoord.setGeo({52.5f, 13.28f});
        Airport sxfCoord;
        sxfCoord.setGeo({52.3f, 13.52f});

        QVERIFY(LocationUtil::isSameLocation(txlCoord, sxfCoord, LocationUtil::CityLevel));
        QVERIFY(LocationUtil::isSameLocation(sxfCoord, txlCoord, LocationUtil::CityLevel));
        QVERIFY(LocationUtil::isSameLocation(txlCoord, txlCoord, LocationUtil::CityLevel));
        QVERIFY(!LocationUtil::isSameLocation(txlCoord, {}, LocationUtil::CityLevel));
        QVERIFY(!LocationUtil::isSameLocation({}, sxfCoord, LocationUtil::CityLevel));

        QVERIFY(!LocationUtil::isSameLocation(sxfCoord, txlCoord, LocationUtil::Exact));
        QVERIFY(LocationUtil::isSameLocation(txlCoord, txlCoord, LocationUtil::Exact));
        QVERIFY(!LocationUtil::isSameLocation(txlCoord, {}, LocationUtil::Exact));
        QVERIFY(!LocationUtil::isSameLocation({}, sxfCoord, LocationUtil::Exact));

        PostalAddress addr;
        addr.setAddressCountry(_("DE"));
        addr.setAddressLocality(_("Berlin"));
        Airport txlAddress;
        txlAddress.setAddress(addr);
        Airport sxfAddress;
        sxfAddress.setAddress(addr);

        QVERIFY(LocationUtil::isSameLocation(txlAddress, sxfAddress, LocationUtil::CityLevel));
        QVERIFY(LocationUtil::isSameLocation(sxfAddress, txlAddress, LocationUtil::CityLevel));
        QVERIFY(LocationUtil::isSameLocation(sxfAddress, sxfAddress, LocationUtil::CityLevel));
        QVERIFY(!LocationUtil::isSameLocation(txlAddress, {}, LocationUtil::CityLevel));
        QVERIFY(!LocationUtil::isSameLocation({}, sxfAddress, LocationUtil::CityLevel));

        QVERIFY(!LocationUtil::isSameLocation(txlAddress, sxfAddress, LocationUtil::Exact));
        QVERIFY(!LocationUtil::isSameLocation(txlAddress, {}, LocationUtil::Exact));
        QVERIFY(!LocationUtil::isSameLocation({}, sxfAddress, LocationUtil::Exact));

        Airport txlIata;
        txlIata.setIataCode(_("TXL"));
        QVERIFY(LocationUtil::isSameLocation(txlIata, txlIata, LocationUtil::CityLevel));
        QVERIFY(LocationUtil::isSameLocation(txlIata, txlIata, LocationUtil::Exact));
        QVERIFY(!LocationUtil::isSameLocation({}, txlIata, LocationUtil::CityLevel));
        QVERIFY(!LocationUtil::isSameLocation(txlIata, {}, LocationUtil::Exact));

        Place txlName;
        txlName.setName(_("Berlin Tegel Airpot"));
        QVERIFY(LocationUtil::isSameLocation(txlName, txlName, LocationUtil::CityLevel));
        QVERIFY(LocationUtil::isSameLocation(txlName, txlName, LocationUtil::Exact));
        QVERIFY(!LocationUtil::isSameLocation({}, txlName, LocationUtil::CityLevel));
        QVERIFY(!LocationUtil::isSameLocation(txlName, {}, LocationUtil::Exact));

        Airport txl;
        txl.setGeo(txlCoord.geo());
        txl.setIataCode(_("TXL"));
        txl.setName(_("Berlin Tegel Airport"));
        Airport sxf;
        sxf.setGeo(sxfCoord.geo());
        sxf.setIataCode(_("SXF"));
        sxf.setName(_("Berlin Schönefeld Airport"));
        QVERIFY(LocationUtil::isSameLocation(txl, sxf, LocationUtil::CityLevel));
        QVERIFY(!LocationUtil::isSameLocation(txl, sxf, LocationUtil::Exact));
        QVERIFY(!LocationUtil::isSameLocation(txl, {}, LocationUtil::CityLevel));
        QVERIFY(!LocationUtil::isSameLocation({}, sxf, LocationUtil::Exact));
    }

    void testLocationNameCompare_data()
    {
        QTest::addColumn<QString>("lhsName");
        QTest::addColumn<QString>("rhsName");
        QTest::addColumn<bool>("cityEqual");
        QTest::addColumn<bool>("exactEqual");

        QTest::newRow("empty") << QString() << QString() << false << false;
        QTest::newRow("equal") << QStringLiteral("Berlin") << QStringLiteral("Berlin") << true << true;
        QTest::newRow("not equal") << QStringLiteral("Nürnberg") << QStringLiteral("Valencia") << false << false;
        QTest::newRow("case-insensitive") << QStringLiteral("Berlin") << QStringLiteral("BERLiN") << true << true;
        QTest::newRow("diacritic") << QStringLiteral("Düsseldorf") << QStringLiteral("Dusseldorf") << true << true;
        QTest::newRow("diacritic 2") << QStringLiteral("Rīga") << QStringLiteral("Riga") << true << true;
        QTest::newRow("diacritic case-insensitive") << QStringLiteral("København H") << QStringLiteral("KOEBENHAVN H") << true << true;
        QTest::newRow("diacritic transliteration") << QStringLiteral("Zürich") << QStringLiteral("ZUERICH") << true << true;
    }

    void testLocationNameCompare()
    {
        QFETCH(QString, lhsName);
        QFETCH(QString, rhsName);
        QFETCH(bool, cityEqual);
        QFETCH(bool, exactEqual);

        Place lhs;
        lhs.setName(lhsName);
        Place rhs;
        rhs.setName(rhsName);
        QCOMPARE(LocationUtil::isSameLocation(lhs, rhs, LocationUtil::CityLevel), cityEqual);
        QCOMPARE(LocationUtil::isSameLocation(rhs, lhs, LocationUtil::CityLevel), cityEqual);
        QCOMPARE(LocationUtil::isSameLocation(lhs, rhs, LocationUtil::Exact), exactEqual);
        QCOMPARE(LocationUtil::isSameLocation(rhs, lhs, LocationUtil::Exact), exactEqual);
    }
};

QTEST_APPLESS_MAIN(LocationUtilTest)

#include "locationutiltest.moc"
