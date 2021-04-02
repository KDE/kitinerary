/*
  SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KItinerary/ExtractorDocumentNode>
#include <KItinerary/ExtractorDocumentProcessor>
#include <KItinerary/ExtractorEngine>
#include <KItinerary/ExtractorResult>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>
#include <QTest>

using namespace KItinerary;

class StructuredDataExtractorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testExtract_data()
    {
        QTest::addColumn<QString>("inputFile");
        QTest::addColumn<QString>("jsonFile");

        QDir dir(QStringLiteral(SOURCE_DIR "/structureddata"));
        const auto lst = dir.entryList(QStringList(QStringLiteral("*.html")), QDir::Files | QDir::Readable | QDir::NoSymLinks);
        for (const auto &file : lst) {
            const QString refFile = dir.path() + QLatin1Char('/') + file.left(file.size() - 5) + QStringLiteral(".json");
            if (!QFile::exists(refFile)) {
                qDebug() << "reference file" << refFile << "does not exist, skipping test file" << file;
                continue;
            }
            QTest::newRow(file.toLatin1().constData()) << QString(dir.path() + QLatin1Char('/') +  file) << refFile;
        }
    }

    void testExtract()
    {
        QFETCH(QString, inputFile);
        QFETCH(QString, jsonFile);

        QFile f(inputFile);
        QVERIFY(f.open(QFile::ReadOnly));

        ExtractorEngine engine;
        engine.setData(f.readAll());
        auto node = engine.rootDocumentNode();
        node.processor()->preExtract(node, &engine);

        QFile ref(jsonFile);
        QVERIFY(ref.open(QFile::ReadOnly));
        const auto doc = QJsonDocument::fromJson(ref.readAll());
        QVERIFY(doc.isArray());

        auto data = node.result().jsonLdResult();
        if (data != doc.array()) {
            qDebug().noquote() << QJsonDocument(data).toJson();
        }
        QCOMPARE(data, doc.array());
    }
};

QTEST_APPLESS_MAIN(StructuredDataExtractorTest)

#include "structureddataextractortest.moc"
