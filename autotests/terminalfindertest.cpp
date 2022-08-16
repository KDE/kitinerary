/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <text/terminalfinder.cpp>

#include <KItinerary/Flight>
#include <KItinerary/Place>

#include <QTest>

using namespace KItinerary;

#define s(x) QStringLiteral(x)

class TerminalFinderTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testTerminalFindingAtStart_data()
    {
        QTest::addColumn<QString>("input");
        QTest::addColumn<int>("len");
        QTest::addColumn<QString>("terminalName");

        QTest::newRow("empty") << QString() << -1 << QString();
        QTest::newRow("no terminal") << s("Paris Charles de Gaulle") << -1 << QString();
        QTest::newRow("CDG 1") << s("PARIS, FR (CHARLES DE GAULLE), TERMINAL 2E") << -1 << QString();
        QTest::newRow("CDG 2") << s(" (Terminal 2D)") << 14 << s("2D");
        QTest::newRow("CDG 3") << s(" - AEROGARE 2") << 13 << s("2");
        QTest::newRow("LHR") << s("-Terminal 2") << 11 << s("2");
        QTest::newRow("MAD") << s(", TERMINAL 4S") << 13 << s("4S");
        QTest::newRow("DTW") << s(" TERMINAL EM") << 12 << s("EM");
        QTest::newRow("MRS1") << s(" - TERMINAL 1A") << 14 << s("1A");
        QTest::newRow("MRS2") << s(" - TERMINAL 1A\nmore data") << 14 << s("1A");
        QTest::newRow("GTW EN") << s(" (North Terminal)") << 17 << s("North");
        QTest::newRow("GTW DE") << s(" (Terminal Nord)") << 16 << s("Nord");
        QTest::newRow("TXL1") << s("(Terminal C)") << 12 << s("C");
        QTest::newRow("TXL2") << s("(Terminal C)  something else") << 12 << s("C");
        QTest::newRow("DEL1") << s(" T2") << 3 << s("2");
        QTest::newRow("DEL2") << s(" (T3)") << 5 << s("3");
        QTest::newRow("DEL3") << s(" T2 some other text") << 3 << s("2");
        QTest::newRow("DEL4") << s(" (T3) more information") << 5 << s("3");
        QTest::newRow("FRA") << s(", Terminal 2 (FRA)\nMadrid (MAD)") << 12 << s("2");
    }

    void testTerminalFindingAtStart()
    {
        QFETCH(QString, input);
        QFETCH(int, len);
        QFETCH(QString, terminalName);

        TerminalFinder f(u"^", u"(?=\\b|\\s|$)");
        const auto res = f.find(input);
        QCOMPARE(res.name, terminalName);
        QCOMPARE(res.start, len == -1 ? -1 : 0);
        QCOMPARE(res.end, len);
    }
};

QTEST_GUILESS_MAIN(TerminalFinderTest)

#include "terminalfindertest.moc"
