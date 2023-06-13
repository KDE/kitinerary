/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testhelpers.h"

#include <tickettokencomparator_p.h>

#include <QTest>

using namespace KItinerary;

#define s(x) QStringLiteral(x)

class TicketTokenComparatorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testNonEqual_data()
    {
        QTest::addColumn<QVariant>("lhs");
        QTest::addColumn<QVariant>("rhs");

        QTest::newRow("type mismatch") << QVariant(s("123456")) << QVariant(QByteArray("123456"));

        const auto uic = Test::readFile(QStringLiteral(SOURCE_DIR "/uic918-3/valid/Testw_VOW8XG6_A9E8DXL_0.bin"));
        auto uic2 = uic;
        uic2[10] = 'X'; // changes the signature key id
        QTest::newRow("uic-semantic-nonequal") << QVariant(uic) << QVariant(uic2);
    }

    void testNonEqual()
    {
        QFETCH(QVariant, lhs);
        QFETCH(QVariant, rhs);

        QVERIFY(!TicketTokenComparator::isSame(lhs, rhs));
        QVERIFY(!TicketTokenComparator::isSame(rhs, lhs));
    }

    void testEqual_data()
    {
        QTest::addColumn<QVariant>("lhs");
        QTest::addColumn<QVariant>("rhs");

        QTest::newRow("both-empty") << QVariant() << QVariant();
        QTest::newRow("equal-qstring") << QVariant(s("123456")) << QVariant(s("123456"));
        QTest::newRow("equal-qbytearray") << QVariant(QByteArray("123456")) << QVariant(QByteArray("123456"));
        QTest::newRow("empty-qstring") << QVariant(s("123456")) << QVariant();
        QTest::newRow("empty-qbytearray") << QVariant(QByteArray("123456")) << QVariant();

        QTest::newRow("iata-prefix")
            << QVariant(s("M1DESMARAIS/LUC       EAB12C3 YULFRAAC 0834 326J003A0027 167>5321WW1325BAC 0014123456002001412346700100141234789012A0141234567890 1AC AC 1234567890123    4PCYLX58Z^108ABCDEFGH"))
            << QVariant(s("M1DESMARAIS/LUC       EABC123 YULFRAAC 0834 326J001A0025 100"));

        const auto uic = Test::readFile(QStringLiteral(SOURCE_DIR "/uic918-3/valid/Testw_VOW8XG6_A9E8DXL_0.bin"));
        auto uic2 = uic;
        uic2[32] = 0x42; // changes the signature
        QTest::newRow("uic-byte-equal") << QVariant(uic) << QVariant(uic);
        QTest::newRow("uic-semantic-equal") << QVariant(uic) << QVariant(uic2);
    }

    void testEqual()
    {
        QFETCH(QVariant, lhs);
        QFETCH(QVariant, rhs);

        QEXPECT_FAIL("iata-prefix", "IATA BCBP token comparison not implemented yet", Abort);
        QVERIFY(TicketTokenComparator::isSame(lhs, rhs));
        QVERIFY(TicketTokenComparator::isSame(rhs, lhs));
    }
};

QTEST_GUILESS_MAIN(TicketTokenComparatorTest)

#include "tickettokencomparatortest.moc"
