/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <asn1/uperdecoder.h>
#include <asn1/uperdecoder.cpp>
#include <asn1/bitvector.cpp>

#include <QDebug>
#include <QObject>
#include <QTest>

using namespace KItinerary;

class UPERDecoderTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testReadNumbers()
    {
        UPERDecoder d(BitVector(QByteArray::fromHex("723004D580D1845E168AEAE4C2D2D840845CAC5C500550E8")));
        d.seek(20);
        QCOMPARE(d.readConstrainedWholeNumber(1, 32000), 9901);
        QCOMPARE(d.offset(), 35);
        QCOMPARE(d.readConstrainedWholeNumber(2016, 2269), 2022);
        QCOMPARE(d.offset(), 43);
        QCOMPARE(d.readConstrainedWholeNumber(1, 366), 281);
        QCOMPARE(d.offset(), 52);
        QCOMPARE(d.readConstrainedWholeNumber(0, 1440), 559);
        QCOMPARE(d.offset(), 63);
        QCOMPARE(d.readUtf8String(), QLatin1String("Eurail B.V."));
        QCOMPARE(d.offset(), 159);
    }
};

QTEST_APPLESS_MAIN(UPERDecoderTest)

#include "uperdecodertest.moc"
