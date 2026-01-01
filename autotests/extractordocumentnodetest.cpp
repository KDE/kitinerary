/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KItinerary/ExtractorDocumentNode>
#include <KItinerary/ExtractorDocumentNodeFactory>
#include <KItinerary/ExtractorDocumentProcessor>
#include <KItinerary/ExtractorEngine>
#include <KItinerary/PdfDocument>

#include <QFile>
#include <QTest>

using namespace KItinerary;

#define s(x) QStringLiteral(x)

class ExtractorDocumentNodeTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testBasics()
    {
        ExtractorDocumentNodeFactory factory;
        ExtractorDocumentNode node;
        QVERIFY(node.isNull());
        node = {};
        QVERIFY(node.isNull());

        node.setContent(s("a plain text content"));
        node.setMimeType(s("text/plain"));
        node.setContextDateTime(QDateTime::currentDateTime());
        QVERIFY(node.isNull()); // not properly constructed

        auto child = factory.createNode(QVariant::fromValue(QByteArray("data")), u"application/octet-stream");
        QVERIFY(!child.isNull());
        node.appendChild(child);
        QCOMPARE(node.childNodes().size(), 1);
        QCOMPARE(child.parent().mimeType(), QLatin1StringView("text/plain"));
        QVERIFY(child.contextDateTime().isValid());
        QCOMPARE(child.contextDateTime(), node.contextDateTime());

        QVERIFY(node.parent().isNull());
        QVERIFY(node.parent().parent().isNull());
    }

    void testPdfFromData()
    {
        ExtractorEngine engine;

        QFile f(QStringLiteral(SOURCE_DIR "/misc/test.pdf"));
        QVERIFY(f.open(QFile::ReadOnly));
        auto root = engine.documentNodeFactory()->createNode(f.readAll());
        QVERIFY(!root.isNull());
        QCOMPARE(root.mimeType(), QLatin1StringView("application/pdf"));
        QCOMPARE(root.childNodes().size(), 0);
        QVERIFY(root.location().isNull());

        root.processor()->expandNode(root, &engine);
        QCOMPARE(root.childNodes().size(), 5);

        auto c3 = root.childNodes()[4];
        QVERIFY(!c3.isNull());
        QCOMPARE(c3.mimeType(), QLatin1StringView("text/plain"));
        QCOMPARE(c3.content<QString>().trimmed(),
                 QLatin1StringView("This is the first page.\n"
                                   "It contains a PDF 417 barcode.\n"
                                   "This is the second page.\n"
                                   "It contains an Aztec code."));

        auto c1 = root.childNodes()[0];
        QVERIFY(!c1.isNull());
        QCOMPARE(c1.mimeType(), QLatin1StringView("internal/qimage"));
        QVERIFY(!c1.parent().isNull());
        QCOMPARE(c1.content().userType(), qMetaTypeId<QImage>());
        QCOMPARE(c1.childNodes().size(), 1); // barcode already expanded by the PDF processor
        QCOMPARE(c1.location().toInt(), 0);

        c1.processor()->expandNode(c1, &engine);
        QCOMPARE(c1.childNodes().size(), 1);
        auto c11 = c1.childNodes()[0];
        QVERIFY(!c11.isNull());
        QCOMPARE(c11.mimeType(), QLatin1StringView("text/plain"));
        QCOMPARE(c11.content<QString>(),
                 QLatin1StringView(
                     "PDF417 is a stacked linear barcode symbol format used in "
                     "a variety of applications, primarily transport, "
                     "identification cards, and inventory management."));
        QCOMPARE(c11.location().toInt(), 0);
    }

    void testPdfFromContent()
    {
        ExtractorEngine engine;

        QFile f(QStringLiteral(SOURCE_DIR "/misc/test.pdf"));
        QVERIFY(f.open(QFile::ReadOnly));
        std::unique_ptr<PdfDocument> pdf(PdfDocument::fromData(f.readAll()));
        auto root = engine.documentNodeFactory()->createNode(QVariant::fromValue(pdf.get()), u"application/pdf");
        QVERIFY(!root.isNull());
        QCOMPARE(root.mimeType(), QLatin1StringView("application/pdf"));

        root.processor()->expandNode(root, &engine);
        QCOMPARE(root.childNodes().size(), 5);
        auto c2 = root.childNodes()[2];
        QVERIFY(!c2.isNull());
        QCOMPARE(c2.mimeType(), QLatin1StringView("internal/qimage"));
        QCOMPARE(c2.location().toInt(), 1);

        c2.processor()->expandNode(c2, &engine);
        QCOMPARE(c2.childNodes().size(), 1);
        auto c21 = c2.childNodes()[0];
        QVERIFY(!c21.isNull());
        QCOMPARE(c21.mimeType(), QLatin1StringView("text/plain"));
        QCOMPARE(c21.content<QString>(),
                 QLatin1StringView(
                     "This is an example Aztec symbol for Wikipedia."));
        QCOMPARE(c21.location().toInt(), 1);
    }

    void testPdfExternal()
    {
        ExtractorEngine engine;
        engine.setUseSeparateProcess(true);

        QFile f(QStringLiteral(SOURCE_DIR "/misc/test.pdf"));
        QVERIFY(f.open(QFile::ReadOnly));
        auto root = engine.documentNodeFactory()->createNode(f.readAll());
        QVERIFY(!root.isNull());
        QCOMPARE(root.mimeType(), QLatin1StringView("application/pdf"));
        root.processor()->expandNode(root, &engine);
        QCOMPARE(root.childNodes().size(), 0);
    }
};

QTEST_GUILESS_MAIN(ExtractorDocumentNodeTest)

#include "extractordocumentnodetest.moc"
