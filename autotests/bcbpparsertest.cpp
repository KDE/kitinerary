/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "iatabcbpparser.h"
#include <KItinerary/JsonLdDocument>
#include <KItinerary/Organization>
#include <KItinerary/Place>

#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>
#include <QTest>

using namespace KItinerary;

class BcbpParserTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testParserValid_data()
    {
        QTest::addColumn<QString>("message");
        QTest::addColumn<QString>("refFile");

        // example data from IATA resolution 792 version 5 Attachment B (with security sections shortended or omitted)
        QTest::newRow("single leg, mandatory only") << QStringLiteral("M1DESMARAIS/LUC       EABC123 YULFRAAC 0834 326J001A0025 100") << QStringLiteral("iata-resolution792-example1.json");
        QTest::newRow("single leg, all fields") << QStringLiteral("M1DESMARAIS/LUC       EAB12C3 YULFRAAC 0834 326J003A0027 167>5321WW1325BAC 0014123456002001412346700100141234789012A0141234567890 1AC AC 1234567890123    4PCYLX58Z^108ABCDEFGH") << QStringLiteral("iata-resolution792-example2.json");
        QTest::newRow("single leg, partial") << QStringLiteral("M1GRANDMAIRE/MELANIE  EABC123 GVACDGAF 0123 339C002F0025 130>5002A0571234567890  AF AF 1234567890123456    Y^18ABCDEFGH") << QStringLiteral("iata-resolution792-example3.json");
        QTest::newRow("multi leg, all fields") << QStringLiteral("M2DESMARAIS/LUC       EAB12C3 YULFRAAC 0834 326J003A0027 167>5321WW1325BAC 0014123456002001412346700100141234789012A0141234567890 1AC AC 1234567890123    4PCYLX58ZDEF456 FRAGVALH 3664 327C012C0002 12E2A0140987654321 1AC AC 1234567890123    3PCNWQ^108ABCDEFGH") << QStringLiteral("iata-resolution792-example4.json");
        QTest::newRow("multi leg, partial") << QStringLiteral("M2GRANDMAIRE/MELANIE  EABC123 GVACDGAF 0123 339C002F0025 130>5002A0571234567890  AF AF 1234567890123456    YDEF456 CDGDTWNW 0049 339F001A0002 12C2A012098765432101                       2PC ^18ABCDEFGH") << QStringLiteral("iata-resolution792-example5.json");

        // EW misses the 'E' eticket marker (BCBP item 253)
        QTest::newRow("missing eticket indicator") << QStringLiteral("M1DOE/JOHN             XXX007 BRUTXLEW 8103 035Y012C0030 147>1181W 8033BEW 0000000000000291040000000000 0   LH 123456789012345     ") << QStringLiteral("missing-eticket-indicator.json");

        // boarding pass issue date (BCBP item 22)
        QTest::newRow("issue date") << QStringLiteral("M1DOE/JOHN            EXXX007 TXLBRUSN 2588 034Y023D0999 35D>5181WM7034BSN              2A08200000000000 SN LH 123456789012345      *30600000K0902       ") << QStringLiteral("issue-date.json");

        // EasyJet being easy on the standard interpretation
        QTest::newRow("easyjet") << QStringLiteral("M1DOE/JOHN            EABCDEFGMRSLGWEZY8724 99  3C  506  10Axxxxxxxxxx") << QStringLiteral("easyjet.json");

        // Brussels Airlines short codes on booking confirmation
        QTest::newRow("minimal1") << QStringLiteral("M1DOE/JOHN            EXXX007 TXLBRUSN 2592 110Y") << QStringLiteral("minimal.json");
        QTest::newRow("minimal2") << QStringLiteral("M1DOE/JOHN            EXXX007 TXLBRUSN 2592 110") << QStringLiteral("minimal2.json");

        // TAP missing boarding pass issue date
        QTest::newRow("no issue date") << QStringLiteral("M1DOE/JOHN            EXXX007 LISLCGTP 1080 204Y002D0003 35C>2180      B1A              2904712345678900                           *306      09     BRND") << QStringLiteral("tap-missing-issue-date.json");

    }

    void testParserValid()
    {
        QFETCH(QString, message);
        QFETCH(QString, refFile);

        QFile f(QStringLiteral(SOURCE_DIR "/bcbpdata/") + refFile);
        QVERIFY(f.open(QFile::ReadOnly));
        const auto refArray = QJsonDocument::fromJson(f.readAll()).array();
        QVERIFY(!refArray.isEmpty());

        const auto res = IataBcbpParser::parse(message, QDate(2018, 4, 2));
        const auto resJson = JsonLdDocument::toJson(res);

        if (refArray != resJson) {
            qWarning().noquote() << QJsonDocument(resJson).toJson();
        }
        QCOMPARE(resJson, refArray);
    }

    void testParserInvalid_data()
    {
        QTest::addColumn<QString>("message");

        QTest::newRow("empty") << QString();
        QTest::newRow("too short") << QStringLiteral("M1DESMARAIS/LUC       ");
        QTest::newRow("wrong leg count") << QStringLiteral("M2DESMARAIS/LUC       EABC123 YULFRAAC 0834 326J001A0025 100");
        QTest::newRow("too short repeated mandatory section") << QStringLiteral("M1DOE/JOHN            EXXX007 TXLBRUSN 2592 11");
    }

    void testParserInvalid()
    {
        QFETCH(QString, message);
        const auto res = IataBcbpParser::parse(message);
        QVERIFY(res.isEmpty());
    }
};

QTEST_APPLESS_MAIN(BcbpParserTest)

#include "bcbpparsertest.moc"
