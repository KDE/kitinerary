/*
  Copyright (c) 2017 Volker Krause <vkrause@kde.org>

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   This library is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
   License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.
*/

#include "extractor.h"
#include "extractorengine.h"
#include "extractorpreprocessor.h"
#include "jsonlddocument.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QTest>

using namespace KItinerary;

class UnstructuredDataExtractorTest : public QObject
{
    Q_OBJECT
private:
    bool loadExtractor(Extractor &extractor, const QString &extractorName)
    {
        QFile f(QLatin1String(":/org.kde.pim/kitinerary/extractors/") + extractorName + QLatin1String(".json"));
        if (!f.open(QFile::ReadOnly)) {
            return false;
        }
        const auto doc = QJsonDocument::fromJson(f.readAll());
        if (doc.isObject()) {
            return extractor.load(doc.object(), QLatin1String(":/org.kde.pim/kitinerary/extractors/"));
        } else if (doc.isArray()) {
            return extractor.load(doc.array().at(0).toObject(), QLatin1String(":/org.kde.pim/kitinerary/extractors/"));
        }
        return false;
    }

private Q_SLOTS:
    void initTestCase()
    {
        // use some exotic locale to ensure the date/time parsing doesn't just work by luck
        QLocale::setDefault(QLocale(QStringLiteral("fr_FR")));
    }

    void testExtractText_data()
    {
        QTest::addColumn<QString>("inputFile");
        QTest::addColumn<QString>("extractorName");
        QTest::addColumn<QString>("jsonFile");

        QDir dir(QStringLiteral(SOURCE_DIR "/unstructureddata"));
        const auto lst = dir.entryList(QStringList(QStringLiteral("*.txt")), QDir::Files | QDir::Readable | QDir::NoSymLinks);
        for (const auto &file : lst) {
            const QString refFile = dir.path() + QLatin1Char('/') + file.left(file.size() - 4) + QStringLiteral(".json");
            if (!QFile::exists(refFile)) {
                qDebug() << "reference file" << refFile << "does not exist, skipping test file" << file;
                continue;
            }
            const auto idx = file.indexOf(QLatin1Char('_'));
            QTest::newRow(file.toLatin1().constData()) << QString(dir.path() + QLatin1Char('/') +  file) << file.left(idx) << refFile;
        }
    }

    void testExtractText()
    {
        QFETCH(QString, inputFile);
        QFETCH(QString, extractorName);
        QFETCH(QString, jsonFile);

        QFile f(inputFile);
        QVERIFY(f.open(QFile::ReadOnly));

        Extractor extractor;
        QVERIFY(loadExtractor(extractor, extractorName));

        ExtractorEngine engine;
        engine.setText(QString::fromUtf8(f.readAll()));
        engine.setSenderDate(QDateTime(QDate(2017, 12, 29), QTime(18, 46, 2)));
        engine.setExtractor(&extractor);
        const auto data = JsonLdDocument::toJson(JsonLdDocument::fromJson(engine.extract()));

        QFile ref(jsonFile);
        QVERIFY(ref.open(QFile::ReadOnly));
        const auto doc = QJsonDocument::fromJson(ref.readAll());
        QVERIFY(doc.isArray());

        if (data != doc.array()) {
            qDebug().noquote() << QJsonDocument(data).toJson();
        }
        QCOMPARE(data, doc.array());
    }

    void testExtractHtml_data()
    {
        QTest::addColumn<QString>("inputFile");
        QTest::addColumn<QString>("extractorName");
        QTest::addColumn<QString>("jsonFile");

        QDir dir(QStringLiteral(SOURCE_DIR "/unstructureddata"));
        const auto lst = dir.entryList(QStringList(QStringLiteral("*.html")), QDir::Files | QDir::Readable | QDir::NoSymLinks);
        for (const auto &file : lst) {
            const QString refFile = dir.path() + QLatin1Char('/') + file.left(file.size() - 5) + QStringLiteral(".json");
            if (!QFile::exists(refFile)) {
                qDebug() << "reference file" << refFile << "does not exist, skipping test file" << file;
                continue;
            }
            const auto idx = file.indexOf(QLatin1Char('_'));
            QTest::newRow(file.toLatin1().constData()) << QString(dir.path() + QLatin1Char('/') +  file) << file.left(idx) << refFile;
        }
    }

    void testExtractHtml()
    {
        QFETCH(QString, inputFile);
        QFETCH(QString, extractorName);
        QFETCH(QString, jsonFile);

        QFile f(inputFile);
        QVERIFY(f.open(QFile::ReadOnly));

        Extractor extractor;
        QVERIFY(loadExtractor(extractor, extractorName));

        ExtractorPreprocessor preproc;
        preproc.preprocessHtml(QString::fromUtf8(f.readAll()));

        ExtractorEngine engine;
        engine.setText(preproc.text());
        engine.setSenderDate(QDateTime(QDate(2017, 12, 29), QTime(18, 46, 2)));
        engine.setExtractor(&extractor);
        const auto data = JsonLdDocument::toJson(JsonLdDocument::fromJson(engine.extract()));

        QFile ref(jsonFile);
        QVERIFY(ref.open(QFile::ReadOnly));
        const auto doc = QJsonDocument::fromJson(ref.readAll());
        QVERIFY(doc.isArray());

        if (data != doc.array()) {
            qDebug().noquote() << QJsonDocument(data).toJson();
        }
        QCOMPARE(data, doc.array());
    }
};

QTEST_MAIN(UnstructuredDataExtractorTest)

#include "unstructureddataextractortest.moc"
