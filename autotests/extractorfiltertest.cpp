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

#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
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
        QCOMPARE(root.mimeType(), QLatin1String("text/calendar"));
        root.processor()->expandNode(root, &engine);
        QCOMPARE(root.childNodes().size(), 1);

        ExtractorFilter filter;
        filter.setMimeType(s("text/calendar"));
        filter.setFieldName(s("productId"));
        filter.setPattern(s("KDE"));
        filter.setScope(ExtractorFilter::Current);

        auto triggerNode = filter.matches(root);
        QVERIFY(triggerNode.isNull());

        filter.setPattern(s("libkcal"));
        triggerNode = filter.matches(root);
        QVERIFY(!triggerNode.isNull());
        QCOMPARE(triggerNode.mimeType(), root.mimeType());

        filter.setScope(ExtractorFilter::Children);
        triggerNode = filter.matches(root);
        QVERIFY(triggerNode.isNull());
        filter.setMimeType(s("internal/event"));
        filter.setPattern(s("Akademy"));
        triggerNode = filter.matches(root);
        QVERIFY(triggerNode.isNull());
        filter.setFieldName(s("summary"));
        triggerNode = filter.matches(root);
        QVERIFY(!triggerNode.isNull());
        QCOMPARE(triggerNode.mimeType(), QLatin1String("internal/event"));
        filter.setScope(ExtractorFilter::Descendants);
        triggerNode = filter.matches(root);
        QVERIFY(!triggerNode.isNull());
        QCOMPARE(triggerNode.mimeType(), QLatin1String("internal/event"));
    }

    void testPkPassFilter()
    {
        QFile f(s(SOURCE_DIR "/pkpassdata/swiss.pkpass"));
        QVERIFY(f.open(QFile::ReadOnly));

        ExtractorEngine engine;
        auto root = engine.documentNodeFactory()->createNode(f.readAll());
        QVERIFY(!root.isNull());
        QCOMPARE(root.mimeType(), QLatin1String("application/vnd.apple.pkpass"));
        root.processor()->expandNode(root, &engine);
        QCOMPARE(root.childNodes().size(), 1);
        auto bcbp = root.childNodes()[0];
        QCOMPARE(bcbp.mimeType(), QLatin1String("text/plain"));

        ExtractorFilter filter;
        filter.setMimeType(s("application/vnd.apple.pkpass"));
        filter.setFieldName(s("passTypeIdentifier"));
        filter.setPattern(s("pass.booking.swiss.com"));

        filter.setScope(ExtractorFilter::Current);
        QVERIFY(filter.matches(bcbp).isNull());
        QVERIFY(!filter.matches(root).isNull());

        filter.setScope(ExtractorFilter::Parent);
        QVERIFY(!filter.matches(bcbp).isNull());
        QVERIFY(filter.matches(root).isNull());
        filter.setScope(ExtractorFilter::Ancestors);
        QVERIFY(!filter.matches(bcbp).isNull());
        QVERIFY(filter.matches(root).isNull());
    }

    void testResultFilter()
    {
        QFile f(s(SOURCE_DIR "/calendarhandlerdata/event.json"));
        QVERIFY(f.open(QFile::ReadOnly));

        ExtractorEngine engine;
        auto root = engine.documentNodeFactory()->createNode(f.readAll());
        QVERIFY(!root.isNull());
        QCOMPARE(root.mimeType(), QLatin1String("application/ld+json"));
        root.processor()->preExtract(root, &engine);
        QCOMPARE(root.result().size(), 1);

        ExtractorFilter filter;
        filter.setMimeType(s("application/ld+json"));
        filter.setFieldName(s("location.address.addressLocality"));
        filter.setPattern(s("Berlin"));
        filter.setScope(ExtractorFilter::Current);
        QVERIFY(filter.matches(root).isNull());
        filter.setPattern(s("Milan"));
        QVERIFY(!filter.matches(root).isNull());
    }
};

QTEST_GUILESS_MAIN(ExtractorFilterTest)

#include "extractorfiltertest.moc"
