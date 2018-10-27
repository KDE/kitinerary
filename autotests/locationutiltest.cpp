/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
};

QTEST_APPLESS_MAIN(LocationUtilTest)

#include "locationutiltest.moc"
