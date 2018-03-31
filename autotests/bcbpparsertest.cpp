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
#include "jsonlddocument.h"
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
    void initTestCase()
    {
        qRegisterMetaType<Airport>();
    }

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
    }

    void testParserValid()
    {
        QFETCH(QString, message);
        QFETCH(QString, refFile);

        QFile f(QStringLiteral(SOURCE_DIR "/bcbpdata/") + refFile);
        QVERIFY(f.open(QFile::ReadOnly));
        const auto refArray = QJsonDocument::fromJson(f.readAll()).array();
        QVERIFY(!refArray.isEmpty());

        const auto res = IataBcbpParser::parse(message);
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
