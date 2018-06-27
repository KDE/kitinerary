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

#include <../src/jsapi/jsonld.h>

#include <QDebug>
#include <QObject>
#include <QTest>

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
};

QTEST_APPLESS_MAIN(JsApiTest)

#include "jsapitest.moc"
