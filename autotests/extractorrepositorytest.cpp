/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KItinerary/AbstractExtractor>
#include <KItinerary/ExtractorDocumentNode>
#include <KItinerary/ExtractorDocumentNodeFactory>
#include <KItinerary/ExtractorEngine>
#include <KItinerary/ExtractorRepository>
#include <KItinerary/ScriptExtractor>

#include <QObject>
#include <QTest>

using namespace KItinerary;

class ExtractorRepositoryTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testReload()
    {
        ExtractorRepository repo;
        const auto count = repo.extractors().size();
        QVERIFY(count > 0);
        QVERIFY(repo.extractorByName(u"chaos-communication-congress"));
        QVERIFY(!repo.extractorByName(u"I-DONT-EXIST"));

        repo.reload();
        QCOMPARE(repo.extractors().size(), count);
        QVERIFY(repo.extractorByName(u"chaos-communication-congress"));
        QVERIFY(!repo.extractorByName(u"I-DONT-EXIST"));
    }

    void testExtractorsForNode()
    {
        ExtractorEngine engine;
        std::vector<const AbstractExtractor*> extractors;

        auto root = engine.documentNodeFactory()->createNode(QStringLiteral("PNR:1234567890,TRAIN:12345,DOJ:dd-mm-yyyy,TIME:hh:mm,SL,A TO B,John Doe+2,S7 49 S7 52 S7 55,FARE:140,SC:10+PG CHGS."), u"text/plain");
        QVERIFY(!root.isNull());
        engine.extractorRepository()->extractorsForNode(root, extractors);
        QCOMPARE(extractors.size(), 1);
        QVERIFY(extractors[0]->name().startsWith(QLatin1StringView("irctc")));
        extractors.clear();

        root = engine.documentNodeFactory()->createNode(QStringLiteral("i0CVxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxX"), u"text/plain");
        QVERIFY(!root.isNull());
        engine.extractorRepository()->extractorsForNode(root, extractors);
        QCOMPARE(extractors.size(), 1);
        QVERIFY(extractors[0]->name().startsWith(QLatin1StringView("sncf")));
        QVERIFY(dynamic_cast<const ScriptExtractor*>(extractors[0]));
        extractors.clear();

        root = engine.documentNodeFactory()->createNode(QStringLiteral("M1DOE/JOHN            EXXX007 TXLBRUSN 2592 110"), u"text/plain");
        QVERIFY(!root.isNull());
        engine.extractorRepository()->extractorsForNode(root, extractors);
        QCOMPARE(extractors.size(), 0);
    }
};

QTEST_GUILESS_MAIN(ExtractorRepositoryTest)

#include "extractorrepositorytest.moc"
