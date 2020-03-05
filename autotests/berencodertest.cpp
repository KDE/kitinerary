/*
    Copyright (C) 2020 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <tlv/berelement.cpp>

#include <QBuffer>
#include <QDebug>
#include <QObject>
#include <QTest>

using namespace KItinerary;

class BerEncoderTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testWriteSize_data()
    {
        QTest::addColumn<int>("size");
        QTest::addColumn<QByteArray>("expected");

        QTest::newRow("zero") << 0 << QByteArray::fromHex("00");
        QTest::newRow("1") << 1 << QByteArray::fromHex("01");
        QTest::newRow("127") << 127 << QByteArray::fromHex("7F");
        QTest::newRow("128") << 128 << QByteArray::fromHex("8180");
        QTest::newRow("255") << 255 << QByteArray::fromHex("81FF");
        QTest::newRow("256") << 256 << QByteArray::fromHex("820100");
        QTest::newRow("65535") << 65535 << QByteArray::fromHex("82FFFF");
    }

    void testWriteSize()
    {
        QFETCH(int, size);
        QFETCH(QByteArray, expected);

        QByteArray out;
        QBuffer buffer(&out);
        QVERIFY(buffer.open(QIODevice::WriteOnly));
        BER::Element::writeSize(&buffer, size);
        buffer.close();
        QCOMPARE(out, expected);
    }
};

QTEST_APPLESS_MAIN(BerEncoderTest)

#include "berencodertest.moc"
