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

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QObject>
#include <QTest>

class UnstructuredDataExtractorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        Q_INIT_RESOURCE(extractors);
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
            const auto refFile = dir.path() + QLatin1Char('/') + file.left(file.size() - 4) + QStringLiteral(".json");
            if (!QFile::exists(refFile)) {
                qDebug() << "reference file" << refFile << "does not exist, skipping test file" << file;
                continue;
            }
            const auto idx = file.indexOf(QLatin1Char('_'));
            QTest::newRow(file.toLatin1()) << QString(dir.path() + QLatin1Char('/') +  file) << file.left(idx) << refFile;
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
        QVERIFY(extractor.load(QLatin1String(":/org.kde.pim/messageviewer/semantic/extractors/") + extractorName + QLatin1String(".json")));

        ExtractorEngine engine;
        engine.setText(QString::fromUtf8(f.readAll()));
        engine.setExtractor(&extractor);
        const auto data = engine.extract();

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
            const auto refFile = dir.path() + QLatin1Char('/') + file.left(file.size() - 5) + QStringLiteral(".json");
            if (!QFile::exists(refFile)) {
                qDebug() << "reference file" << refFile << "does not exist, skipping test file" << file;
                continue;
            }
            const auto idx = file.indexOf(QLatin1Char('_'));
            QTest::newRow(file.toLatin1()) << QString(dir.path() + QLatin1Char('/') +  file) << file.left(idx) << refFile;
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
        QVERIFY(extractor.load(QLatin1String(":/org.kde.pim/messageviewer/semantic/extractors/") + extractorName + QLatin1String(".json")));

        ExtractorPreprocessor preproc;
        preproc.preprocessHtml(QString::fromUtf8(f.readAll()));

        ExtractorEngine engine;
        engine.setText(preproc.text());
        engine.setExtractor(&extractor);
        const auto data = engine.extract();

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
