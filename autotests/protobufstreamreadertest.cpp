/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "../lib/protobuf/protobufstreamreader.cpp"

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

        QTest::newRow("empty") << QByteArray() << 0ul;
        QTest::newRow("0") << QByteArray("\x00", 1) << 0ul;
        QTest::newRow("150") << QByteArray("\x96\x01") << 150ul;
        QTest::newRow("7") << QByteArray("\x07\01\00", 3) << 7ul;
        QTest::newRow("1579970760000") << QByteArray("\xC0\x9A\xE2\xEC\xFD\x2D") << 1579970760000ul;
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

        QTest::newRow("number") << QByteArray::fromHex("089601") << 1ul << ProtobufStreamReader::VARINT;
        QTest::newRow("string-testing") << QByteArray::fromHex("120774657374696e67") << 2ul << ProtobufStreamReader::LEN;
        QTest::newRow("sub-message") << QByteArray::fromHex("1a03089601") << 3ul << ProtobufStreamReader::LEN;
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
    }

    void testLengthDelimitedRecord()
    {
        const auto stringData = QByteArray::fromHex("120774657374696e67");
        ProtobufStreamReader r(std::string_view(stringData.constData(), stringData.size()));
        QCOMPARE(r.readString(), QLatin1String("testing"));

        const auto subMessage = QByteArray::fromHex("1a03089601");
        r = ProtobufStreamReader(std::string_view(subMessage.constData(), subMessage.size()));
        auto subR = r.readSubMessage();
        QCOMPARE(subR.readVarint(), 8);
        QCOMPARE(subR.readVarint(), 150);
    }
};

QTEST_GUILESS_MAIN(ProtobufStreamReaderTest)

#include "protobufstreamreadertest.moc"
