/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "../lib/protobuf/protobufstreamreader.h"

#include <QDebug>
#include <QObject>
#include <QTest>

using namespace KItinerary;

Q_DECLARE_METATYPE(KItinerary::ProtobufStreamReader::WireType)

class ProtobufStreamReaderTest: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testVarint_data()
    {
        QTest::addColumn<QByteArray>("input");
        QTest::addColumn<uint64_t>("output");

        QTest::newRow("empty") << QByteArray() << (uint64_t)0;
        QTest::newRow("0") << QByteArray("\x00", 1) << (uint64_t)0;
        QTest::newRow("150") << QByteArray("\x96\x01") << (uint64_t)150;
        QTest::newRow("7") << QByteArray("\x07\01\00", 3) << (uint64_t)7;
        QTest::newRow("1579970760000") << QByteArray("\xC0\x9A\xE2\xEC\xFD\x2D") << (uint64_t)1579970760000;
    }

    void testVarint()
    {
        QFETCH(QByteArray, input);
        QFETCH(uint64_t, output);

        ProtobufStreamReader r(std::string_view(input.constBegin(), input.size()));
        QCOMPARE(r.peekVarint(), output);
        QCOMPARE(r.peekVarint(), output);
        QCOMPARE(r.readVarint(), output);
    }

    void testFieldRead_data()
    {
        QTest::addColumn<QByteArray>("input");
        QTest::addColumn<uint64_t>("fieldNum");
        QTest::addColumn<ProtobufStreamReader::WireType>("wireType");

        QTest::newRow("number") << QByteArray::fromHex("089601") << (uint64_t)1 << ProtobufStreamReader::VARINT;
        QTest::newRow("string-testing") << QByteArray::fromHex("120774657374696e67") << (uint64_t)2 << ProtobufStreamReader::LEN;
        QTest::newRow("sub-message") << QByteArray::fromHex("1a03089601") << (uint64_t)3 << ProtobufStreamReader::LEN;
    }

    void testFieldRead()
    {
        QFETCH(QByteArray, input);
        QFETCH(uint64_t, fieldNum);
        QFETCH(ProtobufStreamReader::WireType, wireType);

        ProtobufStreamReader r(std::string_view(input.constBegin(), input.size()));
        QCOMPARE(r.fieldNumber(), fieldNum);
        QCOMPARE(r.wireType(), wireType);
        QCOMPARE(r.fieldNumber(), fieldNum);
        QCOMPARE(r.wireType(), wireType);

        QVERIFY(!r.atEnd());
        r.skip();
        QVERIFY(r.atEnd());
    }

    void testLengthDelimitedRecord()
    {
        const auto stringData = QByteArray::fromHex("120774657374696e67");
        ProtobufStreamReader r(std::string_view(stringData.constData(), stringData.size()));
        QCOMPARE(r.readString(), QLatin1StringView("testing"));

        const auto subMessage = QByteArray::fromHex("1a03089601");
        r = ProtobufStreamReader(std::string_view(subMessage.constData(), subMessage.size()));
        auto subR = r.readSubMessage();
        QCOMPARE(subR.readVarintField(), 150);
        QVERIFY(subR.atEnd());
        QVERIFY(r.atEnd());
    }
};

QTEST_GUILESS_MAIN(ProtobufStreamReaderTest)

#include "protobufstreamreadertest.moc"
