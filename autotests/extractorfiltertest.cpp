/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KItinerary/ExtractorDocumentNode>
#include <KItinerary/ExtractorDocumentNodeFactory>
#include <KItinerary/ExtractorDocumentProcessor>
#include <KItinerary/ExtractorEngine>
#include <KItinerary/ExtractorFilter>
#include <KItinerary/ExtractorResult>

#include <QFile>
#include <QTest>

using namespace KItinerary;

#define s(x) QStringLiteral(x)

class ExtractorFilterTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testIcalFilter()
    {
        QFile f(s(SOURCE_DIR "/extractordata/ical/eventreservation.ics"));
        QVERIFY(f.open(QFile::ReadOnly));

        ExtractorEngine engine;
        auto root = engine.documentNodeFactory()->createNode(f.readAll());
        QVERIFY(!root.isNull());
        QCOMPARE(root.mimeType(), QLatin1StringView("text/calendar"));
        root.processor()->expandNode(root, &engine);
        QCOMPARE(root.childNodes().size(), 1);

        ExtractorFilter filter;
        filter.setMimeType(s("text/calendar"));
        filter.setFieldName(s("productId"));
        filter.setPattern(s("KDE"));
        filter.setScope(ExtractorFilter::Current);

        QVERIFY(!filter.matches(root));

        filter.setPattern(s("libkcal"));
        QVERIFY(filter.matches(root));

        std::vector<ExtractorDocumentNode> matches;
        filter.allMatches(root, matches);
        QCOMPARE(matches.size(), 1);
        QCOMPARE(matches[0].mimeType(), root.mimeType());

        filter.setScope(ExtractorFilter::Children);
        QVERIFY(!filter.matches(root));
        filter.setMimeType(s("internal/event"));
        filter.setPattern(s("Akademy"));
        QVERIFY(!filter.matches(root));
        filter.setFieldName(s("summary"));
        QVERIFY(filter.matches(root));
        matches.clear();
        filter.allMatches(root, matches);
        QCOMPARE(matches.size(), 1);
        QCOMPARE(matches[0].mimeType(), QLatin1StringView("internal/event"));
        filter.setScope(ExtractorFilter::Descendants);
        QVERIFY(filter.matches(root));
        matches.clear();
        filter.allMatches(root, matches);
        QCOMPARE(matches.size(), 1);
        QCOMPARE(matches[0].mimeType(), QLatin1StringView("internal/event"));
    }

    void testPkPassFilter()
    {
        QFile f(s(SOURCE_DIR "/pkpassdata/swiss.pkpass"));
        QVERIFY(f.open(QFile::ReadOnly));

        ExtractorEngine engine;
        auto root = engine.documentNodeFactory()->createNode(f.readAll());
        QVERIFY(!root.isNull());
        QCOMPARE(root.mimeType(),
                 QLatin1StringView("application/vnd.apple.pkpass"));
        root.processor()->expandNode(root, &engine);
        QCOMPARE(root.childNodes().size(), 1);
        auto bcbp = root.childNodes()[0];
        QCOMPARE(bcbp.mimeType(), QLatin1StringView("internal/iata-bcbp"));

        ExtractorFilter filter;
        filter.setMimeType(s("application/vnd.apple.pkpass"));
        filter.setFieldName(s("passTypeIdentifier"));
        filter.setPattern(s("pass.booking.swiss.com"));

        filter.setScope(ExtractorFilter::Current);
        QVERIFY(!filter.matches(bcbp));
        QVERIFY(filter.matches(root));

        filter.setScope(ExtractorFilter::Parent);
        QVERIFY(filter.matches(bcbp));
        QVERIFY(!filter.matches(root));
        filter.setScope(ExtractorFilter::Ancestors);
        QVERIFY(filter.matches(bcbp));
        QVERIFY(!filter.matches(root));
    }

    void testResultFilter()
    {
        QFile f(s(SOURCE_DIR "/calendarhandlerdata/event.json"));
        QVERIFY(f.open(QFile::ReadOnly));

        ExtractorEngine engine;
        auto root = engine.documentNodeFactory()->createNode(f.readAll());
        QVERIFY(!root.isNull());
        QCOMPARE(root.mimeType(), QLatin1StringView("application/ld+json"));
        root.processor()->preExtract(root, &engine);
        QCOMPARE(root.result().size(), 1);

        ExtractorFilter filter;
        filter.setMimeType(s("application/ld+json"));
        filter.setFieldName(s("location.address.addressLocality"));
        filter.setPattern(s("Berlin"));
        filter.setScope(ExtractorFilter::Current);
        QVERIFY(!filter.matches(root));
        filter.setPattern(s("Milan"));
        QVERIFY(filter.matches(root));
    }

    void testIataBcbpFilter()
    {
        QFile f(s(SOURCE_DIR "/extractordata/synthetic/iata-bcbp-demo.pdf"));
        QVERIFY(f.open(QFile::ReadOnly));

        ExtractorEngine engine;
        engine.setData(f.readAll());
        engine.extract();
        const auto root = engine.rootDocumentNode();
        QVERIFY(!root.isNull());

        ExtractorFilter filter;
        filter.setMimeType(s("internal/iata-bcbp"));
        filter.setScope(ExtractorFilter::Descendants);
        QVERIFY(filter.matches(root));

        std::vector<ExtractorDocumentNode> matches;
        filter.allMatches(root, matches);
        QCOMPARE(matches.size(), 1);

        filter.setFieldName(s("operatingCarrierDesignator"));
        filter.setPattern(s("AK"));
        QVERIFY(filter.matches(root));
    }
};

QTEST_GUILESS_MAIN(ExtractorFilterTest)

#include "extractorfiltertest.moc"
