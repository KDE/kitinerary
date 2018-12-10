/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <KItinerary/PdfDocument>
#include <config-kitinerary.h>

#include <QDebug>
#include <QFile>
#include <QImage>
#include <QObject>
#include <QTest>

using namespace KItinerary;

class PdfDocumentTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testPdfDocument()
    {
        QFile f(QStringLiteral(SOURCE_DIR "/misc/test.pdf"));
        QVERIFY(f.open(QFile::ReadOnly));
#ifdef HAVE_POPPLER
        std::unique_ptr<PdfDocument> doc(PdfDocument::fromData(f.readAll()));
        QVERIFY(doc);
        QCOMPARE(doc->text(), QStringLiteral("This is the first page.\nIt contains a PDF 417 barcode.\nThis is the second page.\nIt contains an Aztec code.\n"));
        QCOMPARE(doc->pageCount(), 2);
        QCOMPARE(doc->property("pages").toList().size(), 2);

        auto page = doc->page(0);
        QCOMPARE(page.text(), QStringLiteral("This is the first page.\nIt contains a PDF 417 barcode.\n"));
        QCOMPARE(page.imageCount(), 1);
        QCOMPARE(PdfPage::staticMetaObject.property(1).readOnGadget(&page).toList().size(), 1);

        QCOMPARE(page.textInRect(0, 0, 1, 0.5), QStringLiteral("This is the first page.\nIt contains a PDF 417 barcode.\n"));
        QCOMPARE(page.textInRect(0, 0.5, 1, 1), QString());

        auto img = page.image(0);
        QCOMPARE(img.width(), 350);
        QCOMPARE(img.height(), 152);
        QCOMPARE(img.image().width(), img.width());
        QCOMPARE(img.image().height(), img.height());

        page = doc->page(1);
        QCOMPARE(page.text(), QStringLiteral("This is the second page.\nIt contains an Aztec code.\n"));
        QCOMPARE(page.imageCount(), 1);
        img = page.image(0);
        QCOMPARE(img.width(), 276);
        QCOMPARE(img.height(), 276);

        QVERIFY(page.imagesInRect(0, 0, 0.5, 1).isEmpty());
        QCOMPARE(page.imagesInRect(0, 0.5, 1, 1).size(), 1);
#endif
    }

    void testInvalidPdfDocument()
    {
        QVERIFY(!PdfDocument::fromData(QByteArray()));
        QVERIFY(!PdfDocument::fromData(QByteArray("HELLO")));

        QFile f(QStringLiteral(SOURCE_DIR "/misc/test.pdf"));
        QVERIFY(f.open(QFile::ReadOnly));
        QVERIFY(!PdfDocument::fromData(f.readAll().left(f.size() / 2)));
    }
};

QTEST_GUILESS_MAIN(PdfDocumentTest)

#include "pdfdocumenttest.moc"
