/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "../src/lib/jsapi/bitarray.h"

#include <QByteArray>
#include <QObject>
#include <QTest>

using namespace KItinerary::JsApi;

class BitArrayTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testReadNumber()
    {
        BitArray b(QByteArray::fromBase64("IBTCCBAC80gAAAAAAAQH6+9YD9WQiYlJAAAAAAAAZYSaAXLGTgFAAAAAAApgAEigkABAC2UjGEDlwAPyHE5wAAAALw=="));

        QCOMPARE(b.readNumberMSB(0, 8), 0x20);
        QCOMPARE(b.readNumberMSB(0, 16), 0x2014);
        QCOMPARE(b.readNumberMSB(0, 24), 0x2014C2);
        QCOMPARE(b.readNumberMSB(0, 32), 0x2014C208);

        QCOMPARE(b.readNumberMSB(0, 4), 0x2);

        QCOMPARE(b.readNumberMSB(14*8 + 4, 24), 8306421);
        QCOMPARE(b.readNumberMSB(18*8 + 3, 24), 8301700);
        QCOMPARE(b.readNumberMSB(22*8 + 2, 16), 9508);

        QCOMPARE(b.readNumberMSB(31*8 + 2, 7), 9);
        QCOMPARE(b.readNumberMSB(32*8 + 3, 4), 0xD);

        QCOMPARE(b.readNumberMSB(58*8 + 4, 32), 1059177703);
        QCOMPARE(b.readNumberMSB(58*8, 36), 1059177703);

        QCOMPARE(b.readNumberMSB(32*8 + 7, 49), 1592464900416);

        // exactly containing the last bit
        QCOMPARE(b.readNumberMSB(66*8, 8), 0x2F);

        // out of bounds reads
        QCOMPARE(b.readNumberMSB(66*8, 9), 0);
        QCOMPARE(b.readNumberMSB(67*8, 1), 0);
    }
};

QTEST_GUILESS_MAIN(BitArrayTest)

#include "bitarraytest.moc"
