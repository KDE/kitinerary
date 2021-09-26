/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <text/timefinder.cpp>

#include <QDebug>
#include <QTest>

using namespace KItinerary;

#define s(x) QStringLiteral(x)

class TimeFinderTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testTimeFinderSingular_data()
    {
        QTest::addColumn<QString>("input");
        QTest::addColumn<QTime>("time");

        QTest::newRow("ISO") << s("bla 23:42 blub") << QTime(23, 42);
        QTest::newRow("short") << s("1:23") << QTime(1, 23);
        QTest::newRow("min") << s("abc 0:00") << QTime(0, 0);
        QTest::newRow("max") << s("23:59 abc") << QTime(23, 59);
        QTest::newRow("French") << s("bla 16h45 blub") << QTime(16, 45);
        QTest::newRow("dot") << s("bla 18.30 blub") << QTime(18, 30);
        QTest::newRow("US") << s("1:30 pm") << QTime(13, 30);
        QTest::newRow("US midnight") << s("12:00 am") << QTime(0, 0);
        QTest::newRow("US noon") << s("12:00 p.m.") << QTime(12, 0);
        QTest::newRow("extra pm") << s("14:20PM") << QTime(14, 20);
        QTest::newRow("short pm") << s("6:40p") << QTime(18, 40);
        QTest::newRow("Japanese") << s("16時04分") << QTime(16, 4);
        QTest::newRow("dot with ap") << s("12.30 am") << QTime(0, 30);
        QTest::newRow("Korean pm") << s("오후 1시 30분") << QTime(13, 30);
        QTest::newRow("Korean 24h") << s("14시 5분") << QTime(14, 5);
        QTest::newRow("Chinese 24h") << s("19時45分") << QTime(19, 45);
        QTest::newRow("Chinese colon/pm") << s("下午7:45") << QTime(19, 45);
        QTest::newRow("Chinese full/pm") << s("下午7點45分") << QTime(19, 45);
        QTest::newRow("Greek pm") << s("10:40 μ.μ.") << QTime(22, 40);
        QTest::newRow("Arabic pm") << s("11:30م ") << QTime(23, 30);
        // TODO tests for RLM/LRM control chars, and Arabic times with indic numbers
    }

    void testTimeFinderSingular()
    {
        QFETCH(QString, input);
        QFETCH(QTime, time);

        TimeFinder finder;
        finder.find(input);
        QCOMPARE(finder.results().size(), 1);
        QCOMPARE(finder.results()[0].dateTime.toTime(), time);
    }

    void testTimeFinderNone_data()
    {
        QTest::addColumn<QString>("input");

        QTest::newRow("empty") << QString();
        QTest::newRow("text") << s("some text without mentioning a time");
        QTest::newRow("colon") << s("item 3: 24");
        QTest::newRow("DE date") << s("am 12.10. ");
        QTest::newRow("ISO hour out of range") << s("24:59");
        QTest::newRow("ISO min out of range") << s("23:60");
        QTest::newRow("with seconds") << s("12:23:54");
        QTest::newRow("in number") << s("12.234");
        QTest::newRow("price") << s("1.30$");
        QTest::newRow("pm no separator") << s("6:40px");
        QTest::newRow("no leading separation") << s("x12:30 bla");
        QTest::newRow("no trailing separation") << s("\n12:30bla");
        QTest::newRow("ap without separation") << s("07:13ama");
    }

    void testTimeFinderNone()
    {
        QFETCH(QString, input);

        TimeFinder finder;
        finder.find(input);
        QCOMPARE(finder.results().size(), 0);
    }

    void testTimeFinderPlural()
    {
        TimeFinder finder;
        finder.find(u"from 09:00 to 17:00");
        QCOMPARE(finder.results().size(), 2);
        QCOMPARE(finder.results()[0].dateTime.toTime(), QTime(9, 0));
        QCOMPARE(finder.results()[1].dateTime.toTime(), QTime(17, 0));
    }

    void testTimeFinderDateTime_data()
    {
        QTest::addColumn<QString>("input");
        QTest::addColumn<QDateTime>("dateTime");

        QTest::newRow("US") << s("26 Sep 2021 16:43") << QDateTime({2021, 9, 26}, {16, 43});
        QTest::newRow("US") << s("26.09.2021 16:43") << QDateTime({2021, 9, 26}, {16, 43});
    }

    void testTimeFinderDateTime()
    {
        QFETCH(QString, input);
        QFETCH(QDateTime, dateTime);

        TimeFinder finder;
        finder.find(input);
        QCOMPARE(finder.results().size(), 1);
        QCOMPARE(finder.results()[0].dateTime.toDateTime(), dateTime);
    }

};

QTEST_GUILESS_MAIN(TimeFinderTest)

#include "timefindertest.moc"
