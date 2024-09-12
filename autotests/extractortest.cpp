/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testhelpers.h"

#include <KItinerary/ExtractorEngine>
#include <KItinerary/ExtractorPostprocessor>
#include <KItinerary/ExtractorValidator>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/Reservation>
#include <KItinerary/Ticket>

#include <KMime/Message>

#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>
#include <QTest>

void initLocale()
{
    // use some exotic locale to ensure the date/time parsing doesn't just work by luck
    qputenv("LANG", "fr_FR.UTF-8");
}

Q_CONSTRUCTOR_FUNCTION(initLocale)

using namespace KItinerary;

/** Note: this test requires external test data that is not publicly available,
 *  ie. real-world unmodified booking documents.
 *  This data cannot be shared for containing privacy-sensitive data and copyrighted
 *  material (e.g. airline logos).
 */
class ExtractorTest : public QObject
{
    Q_OBJECT
private:
    ExtractorEngine m_engine;

private Q_SLOTS:
    void initTestCase()
    {
        //m_engine.setUseSeparateProcess(true);
    }

    void testExtract_data()
    {
        QTest::addColumn<QString>("contextFile");
        QTest::addColumn<QString>("inputFile");

        for (const QDir baseDir :  {QStringLiteral(SOURCE_DIR "/extractordata"), QStringLiteral(SOURCE_DIR "/../../kitinerary-tests")}) {
            if (!baseDir.exists()) {
                continue;
            }

            QDirIterator it(baseDir.path(), {QStringLiteral("*.txt"), QStringLiteral("*.html"), QStringLiteral("*.pdf"), QStringLiteral("*.pkpass"), QStringLiteral("*.ics"), QStringLiteral("*.eml"), QStringLiteral("*.mbox"), QStringLiteral("*.bin"), QStringLiteral("*.png"), QStringLiteral("*.jpg"), QStringLiteral("*.har"), QStringLiteral("*.in.json"), QStringLiteral("*.gif")}, QDir::Files | QDir::Readable | QDir::NoSymLinks, QDirIterator::Subdirectories);
            while (it.hasNext()) {
                it.next();
                // ignore context files
                if (it.fileName() == QLatin1StringView("context.eml")) {
                  continue;
                }

                QFileInfo contextFi(it.fileInfo().absolutePath() +
                                    QLatin1StringView("/context.eml"));
                QTest::newRow((contextFi.dir().dirName() + QLatin1Char('-') + it.fileName()).toLatin1().constData())
                    << contextFi.absoluteFilePath()
                    << it.fileInfo().absoluteFilePath();
            }
        }
    }

    void testExtract()
    {
        QFETCH(QString, contextFile);
        QFETCH(QString, inputFile);

        m_engine.clear();
        if (inputFile.endsWith(QLatin1StringView(".png")) ||
            inputFile.endsWith(QLatin1StringView(".pdf")) ||
            inputFile.endsWith(QLatin1StringView(".jpg")) ||
            inputFile.endsWith(QLatin1StringView(".gif"))) {
          m_engine.setHints(ExtractorEngine::ExtractFullPageRasterImages);
        } else if (inputFile.endsWith(QLatin1StringView(".ics"))) {
          m_engine.setHints(ExtractorEngine::ExtractGenericIcalEvents);
        } else {
          m_engine.setHints(ExtractorEngine::NoHint);
        }

        QFile inFile(inputFile);
        const auto openFlags = inputFile.endsWith(QLatin1StringView(".txt"))
                                   ? QFile::Text
                                   : QFile::ReadOnly;
        QVERIFY(inFile.open(QFile::ReadOnly | openFlags));

        QFile cf(contextFile);
        KMime::Message contextMsg;
        if (cf.open(QFile::ReadOnly)) {
            contextMsg.setContent(cf.readAll());
            contextMsg.parse();
            m_engine.setContext(QVariant::fromValue(&contextMsg), u"message/rfc822");
        } else if (inputFile.endsWith(QLatin1StringView(".eml"))) {
          contextMsg.setContent(inFile.readAll());
          inFile.seek(0);
          contextMsg.parse();
          m_engine.setContext(QVariant::fromValue(&contextMsg),
                              u"message/rfc822");
        } else {
          m_engine.setContextDate(QDateTime({2018, 1, 1}, {0, 0}));
        }

        m_engine.setData(inFile.readAll(), inputFile);
        auto jsonResult = m_engine.extract();

        const auto expectedSkip =
            QFile::exists(inputFile + QLatin1StringView(".skip"));
        if (jsonResult.isEmpty() && expectedSkip) {
            QSKIP("nothing extracted");
            return;
        }
        QVERIFY(!jsonResult.isEmpty());
        const auto result = JsonLdDocument::fromJson(jsonResult);
        ExtractorPostprocessor postproc;
        postproc.setContextDate(contextMsg.date()->dateTime());
        postproc.process(result);
        auto postProcResult = postproc.result();

        ExtractorValidator validator;
        validator.setAcceptOnlyCompleteElements(false);
        postProcResult.erase(std::remove_if(postProcResult.begin(), postProcResult.end(), [&validator](const auto &elem) {
            return !validator.isValidElement(elem);
        }), postProcResult.end());

        if (postProcResult.isEmpty() && expectedSkip) {
            QSKIP("result filtered");
            return;
        }
        if (postProcResult.isEmpty()) {
            qDebug() << "Result discarded in post processing:";
            qDebug().noquote() << QJsonDocument(jsonResult).toJson();
        }
        QVERIFY(!postProcResult.isEmpty());

        const auto encodedResult = JsonLdDocument::toJson(postProcResult);
        QCOMPARE(encodedResult.size(), postProcResult.size());

        const QString refFile = inputFile + QLatin1StringView(".json");
        if (!QFile::exists(refFile) && !expectedSkip) {
            QFile f(refFile);
            QVERIFY(f.open(QFile::WriteOnly));
            f.write(QJsonDocument(encodedResult).toJson());
            return;
        }

        QFile f(refFile);
        QVERIFY(f.open(QFile::ReadOnly));
        const auto refDoc = QJsonDocument::fromJson(f.readAll());
        QVERIFY(Test::compareJson(refFile, encodedResult, refDoc.array()));

        // verify ticket token prefixes are valid and properly stripped
        for (const auto &res : postProcResult) {
            if (JsonLd::isA<ProgramMembership>(res)) {
                const auto program = res.value<ProgramMembership>();
                if (program.tokenType() == Token::Unknown || program.tokenType() == Token::Url || program.token().isEmpty()) {
                    continue;
                }
                const auto tokenData = program.tokenData();
                if (tokenData.userType() == QMetaType::QString) {
                    QVERIFY(tokenData.toString() != program.token());
                }
                continue;
            }

            Ticket ticket;
            if (JsonLd::canConvert<Reservation>(res)) {
                ticket = JsonLd::convert<Reservation>(res).reservedTicket().value<Ticket>();
            } else if (JsonLd::isA<Ticket>(res)) {
                ticket = res.value<Ticket>();
            } else {
                continue;
            }

            if (ticket.ticketTokenType() == Token::Unknown || ticket.ticketTokenType() == Token::Url || ticket.ticketToken().isEmpty()) {
                continue;
            }
            const auto tokenData = ticket.ticketTokenData();
            if (tokenData.userType() == QMetaType::QString) {
                QVERIFY(tokenData.toString() != ticket.ticketToken());
            }
        }
    }

    void testNegative()
    {
        m_engine.clear();
        m_engine.setData("%PDF-1.4\nINVALID!!!!");
        QCOMPARE(m_engine.extract(), QJsonArray());
    }
};

QTEST_GUILESS_MAIN(ExtractorTest)

#include "extractortest.moc"
