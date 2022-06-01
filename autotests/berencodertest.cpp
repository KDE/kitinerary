/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <tlv/berelement.h>

#include <QBuffer>
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
