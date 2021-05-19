/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "../lib/iata/iatabcbp.h"
#include "../lib/iata/iatabcbpparser.h"

#include <KItinerary/JsonLdDocument>
#include <KItinerary/Organization>
#include <KItinerary/Place>

#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>
#include <QProcess>
#include <QStandardPaths>
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
        QTest::newRow("single leg, mandatory only") << QStringLiteral("M1DESMARAIS/LUC       EABC123 YULFRAAC 0834 326J001A0025 100") << QStringLiteral("iata-resolution792-example1");
        QTest::newRow("single leg, all fields") << QStringLiteral("M1DESMARAIS/LUC       EAB12C3 YULFRAAC 0834 326J003A0027 167>5321WW1325BAC 0014123456002001412346700100141234789012A0141234567890 1AC AC 1234567890123    4PCYLX58Z^108ABCDEFGH") << QStringLiteral("iata-resolution792-example2");
        QTest::newRow("single leg, partial") << QStringLiteral("M1GRANDMAIRE/MELANIE  EABC123 GVACDGAF 0123 339C002F0025 130>5002A0571234567890  AF AF 1234567890123456    Y^108ABCDEFGH") << QStringLiteral("iata-resolution792-example3");
        QTest::newRow("multi leg, all fields") << QStringLiteral("M2DESMARAIS/LUC       EAB12C3 YULFRAAC 0834 326J003A0027 167>5321WW1325BAC 0014123456002001412346700100141234789012A0141234567890 1AC AC 1234567890123    4PCYLX58ZDEF456 FRAGVALH 3664 327C012C0002 12E2A0140987654321 1AC AC 1234567890123    3PCNWQ^108ABCDEFGH") << QStringLiteral("iata-resolution792-example4");
        QTest::newRow("multi leg, partial") << QStringLiteral("M2GRANDMAIRE/MELANIE  EABC123 GVACDGAF 0123 339C002F0025 130>5002A0571234567890  AF AF 1234567890123456    YDEF456 CDGDTWNW 0049 339F001A0002 12C2A012098765432101                       2PC ^108ABCDEFGH") << QStringLiteral("iata-resolution792-example5");

        // EW misses the 'E' eticket marker (BCBP item 253)
        QTest::newRow("missing eticket indicator") << QStringLiteral("M1DOE/JOHN             XXX007 BRUTXLEW 8103 035Y012C0030 147>1181W 8033BEW 0000000000000291040000000000 0   LH 123456789012345     ") << QStringLiteral("missing-eticket-indicator");

        // boarding pass issue date (BCBP item 22)
        QTest::newRow("issue date") << QStringLiteral("M1DOE/JOHN            EXXX007 TXLBRUSN 2588 034Y023D0999 35D>5181WM7034BSN              2A08200000000000 SN LH 123456789012345      *30600000K0902       ") << QStringLiteral("issue-date");

        // EasyJet being easy on the standard interpretation
        QTest::newRow("easyjet") << QStringLiteral("M1DOE/JOHN            EABCDEFGMRSLGWEZY8724 99  3C  506  10Axxxxxxxxxx") << QStringLiteral("easyjet");

        // Brussels Airlines short codes on booking confirmation
        QTest::newRow("minimal1") << QStringLiteral("M1DOE/JOHN            EXXX007 TXLBRUSN 2592 110Y") << QStringLiteral("minimal");
        QTest::newRow("minimal2") << QStringLiteral("M1DOE/JOHN            EXXX007 TXLBRUSN 2592 110") << QStringLiteral("minimal2");

        // TAP missing boarding pass issue date
        QTest::newRow("no issue date") << QStringLiteral("M1DOE/JOHN            EXXX007 LISLCGTP 1080 204Y002D0003 35C>2180      B1A              2904712345678900                           *306      09     BRND") << QStringLiteral("tap-missing-issue-date");

        // Qatar claiming zero-length field sizes
        QTest::newRow("zero size conditional") << QStringLiteral("M1DOE/JANE            EXXX007 MXPDOHQR 0128 256Y042F0023 100>2180  0255BBR              2963456000789980                            0") << QStringLiteral("qatar-zero-size-conditional-section");
    }

    void testParserValid()
    {
        QFETCH(QString, message);
        QFETCH(QString, refFile);

        QVERIFY(IataBcbp::maybeIataBcbp(message));
        QVERIFY(IataBcbp::maybeIataBcbp(message.toUtf8()));
        IataBcbp bcbp(message);
        QVERIFY(bcbp.isValid());

        QFile ref(QLatin1String(SOURCE_DIR "/bcbpdata/") + refFile + QLatin1String(".txt"));
        QVERIFY(ref.open(QFile::ReadOnly));
        const auto refOut = ref.readAll();

        const auto exe = QStandardPaths::findExecutable(QStringLiteral("ticket-barcode-dump"), {QCoreApplication::applicationDirPath()});
        QVERIFY(!exe.isEmpty());
        QProcess proc;
        proc.setProcessChannelMode(QProcess::ForwardedErrorChannel);
        proc.setProgram(exe);
        proc.setArguments({ QLatin1String("--context-date"), QDate(2018, 4, 2).toString(Qt::ISODate) });
        proc.start();
        QVERIFY(proc.waitForStarted());
        proc.write(message.toUtf8());
        QVERIFY(proc.waitForBytesWritten());
        proc.closeWriteChannel();
        QVERIFY(proc.waitForFinished());
        const auto ticketDump = proc.readAllStandardOutput();
        QCOMPARE(proc.exitCode(), 0);
        if (ticketDump != refOut) {
            qDebug().noquote() << ticketDump;
            QFile failFile(ref.fileName() + QLatin1String(".fail"));
            QVERIFY(failFile.open(QFile::WriteOnly));
            failFile.write(ticketDump);
        }
        QVERIFY(ticketDump == refOut);

        QFile f(QLatin1String(SOURCE_DIR "/bcbpdata/") + refFile + QLatin1String(".json"));
        QVERIFY(f.open(QFile::ReadOnly));
        const auto refArray = QJsonDocument::fromJson(f.readAll()).array();
        QVERIFY(!refArray.isEmpty());

        const auto res = IataBcbpParser::parse(bcbp, QDate(2018, 4, 2));
        const auto resJson = JsonLdDocument::toJson(res);

        if (refArray != resJson) {
            qWarning().noquote() << QJsonDocument(resJson).toJson();
            QFile failFile(f.fileName() + QLatin1String(".fail"));
            QVERIFY(failFile.open(QFile::WriteOnly));
            failFile.write(QJsonDocument(resJson).toJson());
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
        QTest::newRow("bug 407895") << QStringLiteral("M2Mobi Lab / ML-586 [In Progress]\nSplit Fooo.Config from main Fooo\n\n========================================================");
    }

    void testParserInvalid()
    {
        QFETCH(QString, message);
        IataBcbp bcbp(message);
        QVERIFY(!bcbp.isValid());
    }
};

QTEST_GUILESS_MAIN(BcbpParserTest)

#include "bcbpparsertest.moc"
