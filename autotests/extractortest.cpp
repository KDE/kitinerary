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

#include <KItinerary/ExtractorEngine>
#include <KItinerary/ExtractorPostprocessor>
#include <KItinerary/ExtractorRepository>
#include <KItinerary/HtmlDocument>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/PdfDocument>

#include <KPkPass/Pass>

#include <KMime/Message>

#include <KCalCore/MemoryCalendar>
#include <KCalCore/ICalFormat>

#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QProcess>
#include <QTest>

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
    ExtractorRepository m_repo;

private Q_SLOTS:
    void initTestCase()
    {
        // use some exotic locale to ensure the date/time parsing doesn't just work by luck
        QLocale::setDefault(QLocale(QStringLiteral("fr_FR")));
    }

    void testExtract_data()
    {
        QTest::addColumn<QString>("contextFile");
        QTest::addColumn<QString>("inputFile");

        QDir baseDir(QStringLiteral(SOURCE_DIR "/../../kitinerary-tests"));
        // test data not available: add dummy entry to not fail the test
        if (!baseDir.exists()) {
            QTest::newRow("test data not available") << QString() << QString();
            return;
        }

        bool someTestsFound = false;

        QDirIterator dirIt(baseDir.path(), {QStringLiteral("context.eml")}, QDir::Files | QDir::Readable | QDir::NoSymLinks, QDirIterator::Subdirectories);
        while (dirIt.hasNext()) {
            QFileInfo contextFi(dirIt.next());
            QDirIterator fileIt(contextFi.absolutePath(), {QStringLiteral("*.txt"), QStringLiteral("*.html"), QStringLiteral("*.pdf"), QStringLiteral("*.pkpass"), QStringLiteral("*.ics")}, QDir::Files | QDir::Readable | QDir::NoSymLinks);
            while (fileIt.hasNext()) {
                fileIt.next();
                someTestsFound = true;
                QTest::newRow((contextFi.dir().dirName() + QLatin1Char('-') + fileIt.fileName()).toLatin1().constData())
                    << contextFi.absoluteFilePath()
                    << fileIt.fileInfo().absoluteFilePath();
            }
        }
        if (!someTestsFound) {
            QTest::newRow("no tests found in test dir") << QString() << QString();
            return;
        }
    }

    void testExtract()
    {
        QFETCH(QString, contextFile);
        QFETCH(QString, inputFile);
        if (contextFile.isEmpty()) {
            return;
        }

        m_engine.clear();

        QFile cf(contextFile);
        QVERIFY(cf.open(QFile::ReadOnly));
        KMime::Message contextMsg;
        contextMsg.setContent(cf.readAll());
        contextMsg.parse();
        m_engine.setSenderDate(contextMsg.date()->dateTime());

        QFile inFile(inputFile);
        QVERIFY(inFile.open(QFile::ReadOnly));

        std::unique_ptr<KPkPass::Pass> pass;
        std::unique_ptr<HtmlDocument> htmlDoc;
        std::unique_ptr<PdfDocument> pdfDoc;
        KCalCore::Calendar::Ptr calendar;
        std::vector<const Extractor*> extractors = m_repo.extractorsForMessage(&contextMsg);
        QJsonArray jsonResult;

        if (inputFile.endsWith(QLatin1String(".pkpass"))) {
            pass.reset(KPkPass::Pass::fromData(inFile.readAll()));
            extractors = m_repo.extractorsForPass(pass.get());
            m_engine.setPass(pass.get());
        } else if (inputFile.endsWith(QLatin1String(".pdf"))) {
            pdfDoc.reset(PdfDocument::fromData(inFile.readAll()));
            QVERIFY(pdfDoc);
            m_engine.setPdfDocument(pdfDoc.get());
        } else if (inputFile.endsWith(QLatin1String(".html"))) {
            htmlDoc.reset(HtmlDocument::fromData(inFile.readAll()));
            QVERIFY(htmlDoc);
            m_engine.setHtmlDocument(htmlDoc.get());
        } else if (inputFile.endsWith(QLatin1String(".txt"))) {
            m_engine.setText(QString::fromUtf8(inFile.readAll()));
        } else if (inputFile.endsWith(QLatin1String(".ics"))) {
            calendar.reset(new KCalCore::MemoryCalendar(QTimeZone()));
            KCalCore::ICalFormat format;
            QVERIFY(format.fromRawString(calendar, inFile.readAll()));
            m_engine.setCalendar(calendar);
        }

        m_engine.setExtractors(std::move(extractors));
        jsonResult = m_engine.extract();

        const auto expectedSkip = QFile::exists(inputFile + QLatin1String(".skip"));
        if (jsonResult.isEmpty() && expectedSkip) {
            QSKIP("nothing extracted");
            return;
        }
        QVERIFY(!jsonResult.isEmpty());
        const auto result = JsonLdDocument::fromJson(jsonResult);
        ExtractorPostprocessor postproc;
        postproc.setContextDate(contextMsg.date()->dateTime());
        postproc.process(result);
        const auto postProcResult = JsonLdDocument::toJson(postproc.result());
        if (postProcResult.isEmpty()) {
            qDebug() << "Result discared in post processing:";
            qDebug().noquote() << QJsonDocument(jsonResult).toJson();
        }
        QVERIFY(!postProcResult.isEmpty());

        const QString refFile = inputFile + QLatin1String(".json");
        if (!QFile::exists(refFile) && !expectedSkip) {
            QFile f(refFile);
            QVERIFY(f.open(QFile::WriteOnly));
            f.write(QJsonDocument(postProcResult).toJson());
            return;
        }

        QFile f(refFile);
        QVERIFY(f.open(QFile::ReadOnly));
        const auto refDoc = QJsonDocument::fromJson(f.readAll());
        if (refDoc.array() != postProcResult) {
            QFile failFile(refFile + QLatin1String(".fail"));
            QVERIFY(failFile.open(QFile::WriteOnly));
            failFile.write(QJsonDocument(postProcResult).toJson());
            failFile.close();

            QProcess proc;
            proc.setProcessChannelMode(QProcess::ForwardedChannels);
            proc.start(QStringLiteral("diff"), {QStringLiteral("-u"), refFile, failFile.fileName()});
            QVERIFY(proc.waitForFinished());
        }

        QCOMPARE(refDoc.array(), postProcResult);
    }
};

QTEST_MAIN(ExtractorTest)

#include "extractortest.moc"
