/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testhelpers.h"

#include <KItinerary/ExtractorDocumentNode>
#include <KItinerary/ExtractorDocumentNodeFactory>
#include <KItinerary/ExtractorDocumentProcessor>
#include <KItinerary/ExtractorEngine>
#include <KItinerary/ExtractorResult>
#include <KItinerary/ScriptExtractor>

#include <kcalendarcore_version.h>

#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QTest>

using namespace KItinerary;

#define s(x) QStringLiteral(x)

void initLocale()
{
    qputenv("LC_ALL", "en_US.utf-8");
    qputenv("TZ", "Europe/Brussels");
}

Q_CONSTRUCTOR_FUNCTION(initLocale)

static void expandRecursive(ExtractorDocumentNode &node, const ExtractorEngine *engine)
{
    if (node.isNull()) {
        return;
    }
    node.processor()->expandNode(node, engine);
    for (auto child : node.childNodes()) {
        expandRecursive(child, engine);
    }

    node.processor()->preExtract(node, engine);
}

class ExtractorScriptEngineTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testArguments_data()
    {
        QTest::addColumn<QString>("inputFile");
        QTest::addColumn<QString>("refFile");

        QTest::newRow("text/plain") << s(SOURCE_DIR "/scriptenginedata/plain-text.txt") << s(SOURCE_DIR "/scriptenginedata/plain-text.txt.json");
        QTest::newRow("pkpass") << s(SOURCE_DIR "/pkpassdata/swiss.pkpass") << s(SOURCE_DIR "/scriptenginedata/swiss.pkpass.json");
        QTest::newRow("IATA BCBP PDF") << s(SOURCE_DIR "/extractordata/synthetic/iata-bcbp-demo.pdf")
            << s(SOURCE_DIR "/scriptenginedata/iata-bcbp-demo.pdf.json");
        QTest::newRow("ical") << s(SOURCE_DIR "/extractordata/ical/eventreservation.ics") << s(SOURCE_DIR "/scriptenginedata/eventreservation.ics.json");
        QTest::newRow("uic9183") << s(SOURCE_DIR "/uic918-3/valid/Testw_VOW8XG6_A9E8DXL_0.bin") << s(SOURCE_DIR "/scriptenginedata/Testw_VOW8XG6_A9E8DXL_0.bin.json");
        QTest::newRow("html") << s(SOURCE_DIR "/structureddata/google-flight-reservation-json-ld.html") << s(SOURCE_DIR "/scriptenginedata/google-flight-reservation-json-ld.html.json");
    }

    void testArguments()
    {
        QFETCH(QString, inputFile);
        QFETCH(QString, refFile);

        QFile in(inputFile);
        const auto openFlags = inputFile.endsWith(QLatin1String(".txt")) ? QFile::Text : QFile::ReadOnly;
        QVERIFY(in.open(QFile::ReadOnly | openFlags));

        ExtractorEngine engine;
        auto root = engine.documentNodeFactory()->createNode(in.readAll(), inputFile);
        QVERIFY(!root.isNull());
        expandRecursive(root, &engine);

        ScriptExtractor extractor;
        extractor.setScriptFileName(s(":/reflector.js"));
        extractor.setScriptFunction(s("dumpArgs"));
        const auto result = extractor.extract(root, &engine).jsonLdResult();

        QFile ref(refFile);
        QVERIFY(ref.open(QFile::ReadOnly));
        const auto refResult = QJsonDocument::fromJson(ref.readAll()).array();
        QVERIFY(Test::compareJson(refFile, result, refResult));
    }

    void testInfiniteLoop()
    {
        QFile in(s(SOURCE_DIR "/scriptenginedata/plain-text.txt"));
        QVERIFY(in.open(QFile::ReadOnly));

        ExtractorEngine engine;
        auto root = engine.documentNodeFactory()->createNode(in.readAll());
        QVERIFY(!root.isNull());
        expandRecursive(root, &engine);

        ScriptExtractor extractor;
        extractor.setScriptFileName(s(":/buggy.js"));
        extractor.setScriptFunction(s("infiniteLoop"));
        const auto result = extractor.extract(root, &engine).jsonLdResult();
    }
};

QTEST_GUILESS_MAIN(ExtractorScriptEngineTest)

#include "extractorscriptenginetest.moc"
