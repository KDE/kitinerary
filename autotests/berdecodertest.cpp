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

#include <QDebug>
#include <QObject>
#include <QTest>

using namespace KItinerary;

class BerDecoderTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testBerElementInvalid_data()
    {
        QTest::addColumn<QByteArray>("input");
        QTest::newRow("empty") << QByteArray();
        QTest::newRow("only type") << QByteArray::fromHex("02");
        QTest::newRow("truncated extended type") << QByteArray::fromHex("1F");
        QTest::newRow("no content") << QByteArray::fromHex("0201");
        QTest::newRow("too large") << QByteArray::fromHex("020242");
        QTest::newRow("variable length no terminator") << QByteArray::fromHex("028042");
    }

    void testBerElementInvalid()
    {
        QFETCH(QByteArray, input);
        BER::Element e(input);
        QVERIFY(!e.isValid());
    }

    void testBerElementType_data()
    {
        QTest::addColumn<QByteArray>("input");
        QTest::addColumn<uint32_t>("type");

        QTest::newRow("primitive type") << QByteArray::fromHex("020142") << 0x02u;
        QTest::newRow("extended type") << QByteArray::fromHex("1F420142") << 0x1F42u;
    }

    void testBerElementType()
    {
        QFETCH(QByteArray, input);
        QFETCH(uint32_t, type);
        BER::Element e(input);
        QVERIFY(e.isValid());
        QCOMPARE(e.type(), type);
        QCOMPARE(e.size(), input.size());
        QVERIFY(e.contentData());
    }

    void testBerElementContentSize_data()
    {
        QTest::addColumn<QByteArray>("input");
        QTest::addColumn<int>("size");

        QTest::newRow("basic size") << QByteArray::fromHex("020142") << 1;
        QTest::newRow("1 byte extended size") << QByteArray::fromHex("02810142") << 1;

        QByteArray b(256, 42);
        b.prepend(QByteArray::fromHex("02820100"));
        QTest::newRow("2 byte extended size") << b << 256;

        QTest::newRow("variable length") << QByteArray::fromHex("0280420000") << 1;
    }

    void testBerElementContentSize()
    {
        QFETCH(QByteArray, input);
        QFETCH(int, size);
        BER::Element e(input);
        QVERIFY(e.isValid());
        QCOMPARE(e.contentSize(), size);
        QCOMPARE(e.size(), input.size());
        QVERIFY(e.contentData());
    }

    void testBerRecursionInvalid_data()
    {
        QTest::addColumn<QByteArray>("input");
        QTest::newRow("incomplete content") << QByteArray::fromHex("1E020201");
        QTest::newRow("wrong child size") << QByteArray::fromHex("1E03020242");
        QTest::newRow("wrong child size with trailing data") << QByteArray::fromHex("1E03020242020142");
        QTest::newRow("variable length child") << QByteArray::fromHex("1E0402804242420000");
    }

    void testBerRecursionInvalid()
    {
        QFETCH(QByteArray, input);
        BER::Element e(input);
        QVERIFY(e.isValid());
        QVERIFY(!e.first().isValid());
    }

    void testBerChildCount_data()
    {
        QTest::addColumn<QByteArray>("input");
        QTest::addColumn<int>("childCount");
        QTest::newRow("one child") << QByteArray::fromHex("1E03020142") << 1;
        QTest::newRow("two children") << QByteArray::fromHex("1E06020142020123") << 2;
        QTest::newRow("one child, trailing element") << QByteArray::fromHex("1E03020142020123") << 1;
    }

    void testBerChildCount()
    {
        QFETCH(QByteArray, input);
        QFETCH(int, childCount);

        BER::Element e(input);
        QVERIFY(e.isValid());

        BER::Element c = e.first();
        QVERIFY(c.isValid());
        int i = 0;
        while (c.isValid()) {
            c = c.next();
            ++i;
        }
        QCOMPARE(i, childCount);
    }

    void testBerFindChild_data()
    {
        QTest::addColumn<QByteArray>("input");
        QTest::addColumn<uint32_t>("type");
        QTest::addColumn<bool>("found");
        QTest::newRow("no children") << QByteArray::fromHex("020142") << 0x02u << false;
        QTest::newRow("children, not found") << QByteArray::fromHex("1E06020142020123") << 0x04u << false;
        QTest::newRow("children, first hit") << QByteArray::fromHex("1E06040142020123") << 0x04u << true;
        QTest::newRow("children, second hit") << QByteArray::fromHex("1E06020142040123") << 0x04u << true;
    }

    void testBerFindChild()
    {
        QFETCH(QByteArray, input);
        QFETCH(uint32_t, type);
        QFETCH(bool, found);

        BER::Element e(input);
        QVERIFY(e.isValid());
        auto c = e.find(type);
        QCOMPARE(c.isValid(), found);
    }
};

QTEST_APPLESS_MAIN(BerDecoderTest)

#include "berdecodertest.moc"
