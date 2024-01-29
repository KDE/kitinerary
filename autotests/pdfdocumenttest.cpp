/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KItinerary/PdfDocument>
#include <KItinerary/PdfLink>

#include <QFile>
#include <QObject>
#include <QTest>

using namespace KItinerary;

#ifdef Q_OS_WINDOWS
#define LINEBREAK "\r\n"
#else
#define LINEBREAK "\n"
#endif

class PdfDocumentTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        qputenv("TZ", "UTC");
    }

    void testPdfDocument()
    {
        QFile f(QStringLiteral(SOURCE_DIR "/misc/test.pdf"));
        QVERIFY(f.open(QFile::ReadOnly));
        QVERIFY(PdfDocument::maybePdf(f.readAll()));
        f.seek(0);
        std::unique_ptr<PdfDocument> doc(PdfDocument::fromData(f.readAll()));
        QVERIFY(doc);
        QCOMPARE(doc->text(),
                 QLatin1StringView("This is the first page." LINEBREAK
                                   "It contains a PDF 417 barcode." LINEBREAK
                                   "This is the second page." LINEBREAK
                                   "It contains an Aztec code." LINEBREAK));
        QCOMPARE(doc->pageCount(), 2);
        QCOMPARE(doc->property("pages").toList().size(), 2);

        auto page = doc->page(0);
        QCOMPARE(page.text(),
                 QLatin1StringView("This is the first page." LINEBREAK
                                   "It contains a PDF 417 barcode." LINEBREAK));
        QCOMPARE(page.imageCount(), 1);
        QCOMPARE(PdfPage::staticMetaObject.property(1).readOnGadget(&page).toList().size(), 1);
        QCOMPARE(page.width(), 210);
        QCOMPARE(page.height(), 296);

        QCOMPARE(page.textInRect(0, 0, 1, 0.5),
                 QLatin1StringView("This is the first page." LINEBREAK
                                   "It contains a PDF 417 barcode." LINEBREAK));
        QCOMPARE(page.textInRect(0, 0.5, 1, 1), QString());

        auto img = page.image(0);
        QCOMPARE(img.width(), 212);
        QCOMPARE(img.height(), 92);
        QCOMPARE(img.sourceHeight(), 152);
        QCOMPARE(img.sourceWidth(), 350);
        QCOMPARE(img.image().width(), 350);
        QCOMPARE(img.image().height(), 152);

        page = doc->page(1);
        QCOMPARE(page.text(),
                 QLatin1StringView("This is the second page." LINEBREAK
                                   "It contains an Aztec code." LINEBREAK));
        QCOMPARE(page.imageCount(), 1);
        img = page.image(0);
        QCOMPARE(img.width(), 93);
        QCOMPARE(img.height(), 93);
        QCOMPARE(img.image().width(), 276);
        QCOMPARE(img.image().height(), 276);
        QCOMPARE(img.sourceHeight(), 276);
        QCOMPARE(img.sourceWidth(), 276);

        QVERIFY(page.imagesInRect(0, 0, 0.5, 1).isEmpty());
        QCOMPARE(page.imagesInRect(0, 0.5, 1, 1).size(), 1);

        QCOMPARE(doc->creationTime(), QDateTime({2018, 4, 29}, {11, 41, 28}, QTimeZone::fromSecondsAheadOfUtc(7200)));
        QCOMPARE(doc->modificationTime(), QDateTime());
    }

    void testPdfLink()
    {
        QFile f(QStringLiteral(SOURCE_DIR "/misc/link.pdf"));
        QVERIFY(f.open(QFile::ReadOnly));
        QVERIFY(PdfDocument::maybePdf(f.readAll()));
        f.seek(0);
        std::unique_ptr<PdfDocument> doc(PdfDocument::fromData(f.readAll()));
        QVERIFY(doc);

        QCOMPARE(doc->pageCount(), 1);
        const auto page = doc->page(0);
        QCOMPARE(page.linkCount(), 1);
        auto link = page.link(0);
        QCOMPARE(link.url(), QLatin1StringView("https://kde.org"));
        qDebug() << link.area();
        QVERIFY(link.area().isValid());

        QCOMPARE(page.linksInRect(0.0, 0.0, 1.0, 1.0).size(), 1);
        QCOMPARE(page.linksInRect(0.0, 0.0, 1.0, 0.5).size(), 1);
        link = page.linksInRect(0.0, 0.0, 0.5, 0.5).at(0).value<PdfLink>();
        QCOMPARE(link.url(), QLatin1StringView("https://kde.org"));
        QCOMPARE(page.linksInRect(0.0, 0.0, 0.2, 0.5).size(), 1);
        QCOMPARE(page.linksInRect(0.0, 0.5, 1.0, 1.0).size(), 0);
    }

    void testInvalidPdfDocument()
    {
        QVERIFY(!PdfDocument::maybePdf(QByteArray()));
        QVERIFY(!PdfDocument::fromData(QByteArray()));
        QVERIFY(!PdfDocument::maybePdf(QByteArray("HELLO")));
        QVERIFY(!PdfDocument::fromData(QByteArray("HELLO")));

        QFile f(QStringLiteral(SOURCE_DIR "/misc/test.pdf"));
        QVERIFY(f.open(QFile::ReadOnly));
        QVERIFY(!PdfDocument::fromData(f.readAll().left(f.size() / 2)));
    }
};

QTEST_GUILESS_MAIN(PdfDocumentTest)

#include "pdfdocumenttest.moc"
