/*
    Copyright (c) 2019 Volker Krause <vkrause@kde.org>

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

#include <../src/jsapi/bitarray.h>

#include <QByteArray>
#include <QDebug>
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
    }
};

QTEST_GUILESS_MAIN(BitArrayTest)

#include "bitarraytest.moc"
