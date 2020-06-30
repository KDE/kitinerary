/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <../src/jsapi/jsonld.h>

#include <QDebug>
#include <QJSEngine>
#include <QJSValue>
#include <QObject>
#include <QTest>

#include <cmath>

using namespace KItinerary;

#define s(x) QStringLiteral(x)

class JsApiTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        // use some exotic locale and timezone to ensure the date/time parsing doesn't just work by luck
        QLocale::setDefault(QLocale(QStringLiteral("fr_FR")));
        qputenv("TZ", "GMT");
    }

    void testToDateTime_data()
    {
        QTest::addColumn<QString>("input");
        QTest::addColumn<QString>("format");
        QTest::addColumn<QString>("locale");
        QTest::addColumn<QDateTime>("result");

        QTest::newRow("iso") << s("2018-06-22 19:37") << s("yyyy-MM-dd hh:mm") << s("en") << QDateTime({2018, 6, 22}, {19, 37});
        QTest::newRow("2 digit year") << s("18-06-22 19:37") << s("yy-MM-dd hh:mm") << s("en") << QDateTime({2018, 6, 22}, {19, 37});
        QTest::newRow("short month name en") << s("2018 Mar 22 19:37") << s("yyyy MMM dd hh:mm") << s("en") << QDateTime({2018, 3, 22}, {19, 37});
        QTest::newRow("short month name sv") << s("2018 Mar 22 19:37") << s("yyyy MMM dd hh:mm") << s("sv_SE") << QDateTime({2018, 3, 22}, {19, 37});
        QTest::newRow("short month name de") << s("2018 Mai 22 19:37") << s("yyyy MMM dd hh:mm") << s("de") << QDateTime({2018, 5, 22}, {19, 37});
        QTest::newRow("missing year") << s("1 22 19:37") << s("M dd hh:mm") << s("en") << QDateTime({2019, 1, 22}, {19, 37});
        QTest::newRow("December short sv") << s("4 dec 2012 07:05") << s("d MMM yyyy hh:mm") << s("sv_SE") << QDateTime({2012, 12, 4}, {7, 5});
        QTest::newRow("time only") << s("19:08") << s("hh:mm") << s("en") << QDateTime({1970, 1, 1}, {19, 8});

        // commonly used abbreviations in de differ from QLocale
        QTest::newRow("short month de 1") << s("2018 Mär 22 19:37") << s("yyyy MMM dd hh:mm") << s("de") << QDateTime({2018, 3, 22}, {19, 37});
        QTest::newRow("short month de 2") << s("2018 Mrz 22 19:37") << s("yyyy MMM dd hh:mm") << s("de") << QDateTime({2018, 3, 22}, {19, 37});
        QTest::newRow("short month de 3") << s("2018 Jun 22 19:37") << s("yyyy MMM dd hh:mm") << s("de") << QDateTime({2018, 6, 22}, {19, 37});
        QTest::newRow("short month de 4") << s("2018 Jul 22 19:37") << s("yyyy MMM dd hh:mm") << s("de") << QDateTime({2018, 7, 22}, {19, 37});
        QTest::newRow("short month de 5") << s("2018 Okt 22 19:37") << s("yyyy MMM dd hh:mm") << s("de") << QDateTime({2018, 10, 22}, {19, 37});
    }

    void testToDateTime()
    {
        QFETCH(QString, input);
        QFETCH(QString, format);
        QFETCH(QString, locale);
        QFETCH(QDateTime, result);

        JsApi::JsonLd jsonLd(nullptr);
        jsonLd.setContextDate({{2018, 4, 1}, {}});
        QCOMPARE(jsonLd.toDateTime(input, format, locale), result);
    }

    void testToGeoCoordinates_data()
    {
        QTest::addColumn<QString>("url");
        QTest::addColumn<double>("latitude");
        QTest::addColumn<double>("longitude");

        QTest::newRow("empty") << QString() << (double)NAN << (double)NAN;
        QTest::newRow("non-map") << s("http://www.kde.org") << (double)NAN << (double)NAN;
        QTest::newRow("google maps 1") << s("https://www.google.com/maps/place/48.182849,16.378636") << 48.182849 << 16.378636;
        QTest::newRow("google maps 1 no ssl") << s("http://www.google.com/maps/place/48.182849,16.378636") << 48.182849 << 16.378636;
        QTest::newRow("google maps 2") << s("http://maps.google.fr/?ll=-48.8471603393555,-2.37761735916138&z=19") << -48.8471603393555 << -2.37761735916138;
        QTest::newRow("google maps 3") << s("https://maps.google.com/maps?f=q&hl=en&q=52.434788,-13.544329") << 52.434788 << -13.544329;
        QTest::newRow("google maps api 1") << s("http://maps.googleapis.com/maps/api/staticmap?language=en-gb&size=280x120&center=69.54363918781344,31.028996706008911&sensor=false&markers=color:0x0896FF%7C69.54363918781344,31.028996706008911&zoom=14&client=gme-booking&signature=xxxxxxxxxxxxxxxxxxxxxx") << 69.54363918781344 << 31.028996706008911;
    }

    void testToGeoCoordinates()
    {
        QFETCH(QString, url);
        QFETCH(double, latitude);
        QFETCH(double, longitude);
        QCOMPARE(std::isnan(latitude), std::isnan(longitude));

        QJSEngine engine;
        JsApi::JsonLd api(&engine);
        const auto geo = api.toGeoCoordinates(url);
        QVERIFY((geo.isUndefined() && std::isnan(latitude)) || (!geo.isUndefined() && !std::isnan(latitude)));
        if (geo.isUndefined()) {
            return;
        }

        QCOMPARE(geo.property(s("latitude")).toNumber(), latitude);
        QCOMPARE(geo.property(s("longitude")).toNumber(), longitude);
    }
};

QTEST_GUILESS_MAIN(JsApiTest)

#include "jsapitest.moc"
