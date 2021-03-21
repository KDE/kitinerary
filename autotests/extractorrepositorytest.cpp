/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KItinerary/AbstractExtractor>
#include <KItinerary/Extractor>
#include <KItinerary/ExtractorDocumentNode>
#include <KItinerary/ExtractorDocumentNodeFactory>
#include <KItinerary/ExtractorEngine>
#include <KItinerary/ExtractorRepository>
#include <KItinerary/ScriptExtractor>

#include <QDebug>
#include <QObject>
#include <QTest>
#include <QVariant>

using namespace KItinerary;

class ExtractorRepositoryTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testReload()
    {
        ExtractorRepository repo;
        const auto count = repo.allExtractors().size();
        QVERIFY(count > 0);
        repo.reload();
        QCOMPARE(repo.allExtractors().size(), count);
    }

    void testApplyFilter()
    {
        ExtractorRepository repo;

        std::vector<Extractor> extractors;
        repo.extractorsForBarcode(QStringLiteral("i0CVxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxX"), extractors);
        QCOMPARE(extractors.size(), 1);
        QVERIFY(extractors[0].name().startsWith(QLatin1String("sncf")));

        extractors.clear();
        repo.extractorsForContent(QStringLiteral("PNR:1234567890,TRAIN:12345,DOJ:dd-mm-yyyy,TIME:hh:mm,SL,A TO B,John Doe+2,S7 49 S7 52 S7 55,FARE:140,SC:10+PG CHGS."), extractors);
        QCOMPARE(extractors.size(), 1);
        QVERIFY(extractors[0].name().startsWith(QLatin1String("irctc")));
    }

    void testExtractorsForNode()
    {
        ExtractorEngine engine;
        std::vector<const AbstractExtractor*> extractors;

        auto root = engine.documentNodeFactory()->createNode(QStringLiteral("PNR:1234567890,TRAIN:12345,DOJ:dd-mm-yyyy,TIME:hh:mm,SL,A TO B,John Doe+2,S7 49 S7 52 S7 55,FARE:140,SC:10+PG CHGS."), u"text/plain");
        QVERIFY(!root.isNull());
        engine.extractorRepository()->extractorsForNode(root, extractors);
        QCOMPARE(extractors.size(), 1);
        QVERIFY(extractors[0]->name().startsWith(QLatin1String("irctc")));
        extractors.clear();

        root = engine.documentNodeFactory()->createNode(QStringLiteral("i0CVxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxX"), u"text/plain");
        QVERIFY(!root.isNull());
        engine.extractorRepository()->extractorsForNode(root, extractors);
        QCOMPARE(extractors.size(), 1);
        QVERIFY(extractors[0]->name().startsWith(QLatin1String("sncf")));
        QVERIFY(dynamic_cast<const ScriptExtractor*>(extractors[0]));
        extractors.clear();

        root = engine.documentNodeFactory()->createNode(QStringLiteral("M1DOE/JOHN            EXXX007 TXLBRUSN 2592 110"), u"text/plain");
        QVERIFY(!root.isNull());
        engine.extractorRepository()->extractorsForNode(root, extractors);
        QCOMPARE(extractors.size(), 1);
        QVERIFY(extractors[0]->name().startsWith(QLatin1String("<IATA BCBP>")));
        QVERIFY(!dynamic_cast<const ScriptExtractor*>(extractors[0]));
        extractors.clear();
    }
};

QTEST_GUILESS_MAIN(ExtractorRepositoryTest)

#include "extractorrepositorytest.moc"
