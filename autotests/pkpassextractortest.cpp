/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KItinerary/Extractor>
#include <KItinerary/ExtractorEngine>
#include <KItinerary/ExtractorPostprocessor>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/Organization>
#include <KItinerary/Place>

#include <KPkPass/BoardingPass>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>
#include <QTest>

using namespace KItinerary;

class PkPassExtractorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        // use some exotic locale to ensure the date/time parsing doesn't just work by luck
        QLocale::setDefault(QLocale(QStringLiteral("fr_FR")));
        qputenv("TZ", "EST");
    }

    void testExtractText_data()
    {
        QTest::addColumn<QString>("inputFile");
        QTest::addColumn<QString>("refFile");

        QDir dir(QStringLiteral(SOURCE_DIR "/pkpassdata"));
        const auto lst = dir.entryList(QStringList(QStringLiteral("*.pkpass")), QDir::Files | QDir::Readable | QDir::NoSymLinks);
        for (const auto &file : lst) {
            const QString refFile = dir.path() + QLatin1Char('/') + file.left(file.size() - 7) + QStringLiteral(".json");
            if (!QFile::exists(refFile)) {
                qDebug() << "reference file" << refFile << "does not exist, skipping test file" << file;
                continue;
            }
            QTest::newRow(file.toLatin1().constData()) << QString(dir.path() + QLatin1Char('/') +  file) << refFile;
        }
    }

    void testExtractText()
    {
        QFETCH(QString, inputFile);
        QFETCH(QString, refFile);

        const auto pass = KPkPass::Pass::fromFile(inputFile, this);
        QVERIFY(pass);

        ExtractorEngine engine;
        engine.setContextDate(QDateTime(QDate(2017, 12, 29), QTime(18, 46, 2)));
        engine.setPass(pass);
        auto result = JsonLdDocument::fromJson(engine.extract());

        ExtractorPostprocessor postproc;
        postproc.setContextDate(QDateTime(QDate(2017, 12, 29), QTime(18, 46, 2)));
        postproc.process(result);
        result = postproc.result();
        QVERIFY(result.size() <= 1);
        const auto resJson = JsonLdDocument::toJson(result);

        QFile ref(refFile);
        QVERIFY(ref.open(QFile::ReadOnly));
        const auto doc = QJsonDocument::fromJson(ref.readAll());
        QVERIFY(doc.isArray());
        QCOMPARE(doc.array().size(), result.size());

        if (resJson != doc.array()) {
            qDebug().noquote() << QJsonDocument(resJson).toJson();
        }
        QCOMPARE(resJson, doc.array());
    }
};

QTEST_GUILESS_MAIN(PkPassExtractorTest)

#include "pkpassextractortest.moc"
