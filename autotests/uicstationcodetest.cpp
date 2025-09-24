/*
    SPDX-FileCopyrightText: ⓒ 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "../src/lib/uic9183/uicstationcode.cpp"

#include <QTest>

using namespace KItinerary;

class UicStationCodeTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testChecksum_data()
    {
        QTest::addColumn<int>("input");
        QTest::addColumn<int>("checksum");

        QTest::newRow("empty") << 0 << -1;
        QTest::newRow("87193615") << 8719361 << 5;
        QTest::newRow("87193250") << 8719325 << 0;
        QTest::newRow("Paris Gare de Nord") << 8727100 << 7;
    }

    void testChecksum()
    {
        QFETCH(int, input);
        QFETCH(int, checksum);
        QCOMPARE(UicStationCode::checksumDigit(input), checksum);
    }
};

QTEST_GUILESS_MAIN(UicStationCodeTest)

#include "uicstationcodetest.moc"
