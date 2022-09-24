/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <config-kitinerary.h>

#include "testhelpers.h"

#include <KItinerary/ExtractorEngine>
#include <KItinerary/ExtractorPostprocessor>
#include <KItinerary/ExtractorValidator>
#include <KItinerary/JsonLdDocument>

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
    qputenv("LANG", "fr_FR");
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

            QDirIterator it(baseDir.path(), {QStringLiteral("*.txt"), QStringLiteral("*.html"), QStringLiteral("*.pdf"), QStringLiteral("*.pkpass"), QStringLiteral("*.ics"), QStringLiteral("*.eml"), QStringLiteral("*.mbox"), QStringLiteral("*.bin"), QStringLiteral("*.png")}, QDir::Files | QDir::Readable | QDir::NoSymLinks, QDirIterator::Subdirectories);
            while (it.hasNext()) {
                it.next();
                // ignore context files
                if (it.fileName() == QLatin1String("context.eml")) {
                    continue;
                }

                QFileInfo contextFi(it.fileInfo().absolutePath() + QLatin1String("/context.eml"));
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
        m_engine.setHints(inputFile.endsWith(QLatin1String(".png")) ? ExtractorEngine::ExtractFullPageRasterImages : ExtractorEngine::NoHint);

        QFile inFile(inputFile);
        QVERIFY(inFile.open(QFile::ReadOnly));

        QFile cf(contextFile);
        KMime::Message contextMsg;
        if (cf.open(QFile::ReadOnly)) {
            contextMsg.setContent(cf.readAll());
            contextMsg.parse();
            m_engine.setContext(QVariant::fromValue(&contextMsg), u"message/rfc822");
        } else if (inputFile.endsWith(QLatin1String(".eml"))) {
            contextMsg.setContent(inFile.readAll());
            inFile.seek(0);
            contextMsg.parse();
            m_engine.setContext(QVariant::fromValue(&contextMsg), u"message/rfc822");
        } else {
            m_engine.setContextDate(QDateTime({2018, 1, 1}, {0, 0}));
        }

        m_engine.setData(inFile.readAll(), inputFile);
        auto jsonResult = m_engine.extract();

        const auto expectedSkip = QFile::exists(inputFile + QLatin1String(".skip"));
        if (jsonResult.isEmpty() && expectedSkip) {
            QSKIP("nothing extracted");
            return;
        }
#if !HAVE_ZXING
        if (jsonResult.isEmpty()) {
            QSKIP("nothing extracted, but ZXing is missing!");
            return;
        }
#endif
        QVERIFY(!jsonResult.isEmpty());
        const auto result = JsonLdDocument::fromJson(jsonResult);
        ExtractorPostprocessor postproc;
        postproc.setContextDate(contextMsg.date()->dateTime());
        postproc.setValidationEnabled(false);
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

        const QString refFile = inputFile + QLatin1String(".json");
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
