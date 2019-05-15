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

#include <config-kitinerary.h>
#include <../src/barcodedecoder_p.h>

#include <QDebug>
#include <QImage>
#include <QObject>
#include <QTest>

using namespace KItinerary;

class BarcodeDecoderTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testPDF417_data()
    {
        QTest::addColumn<QString>("fileName");

        QTest::newRow("1bit") << QStringLiteral("pdf417_1bit.png");
        QTest::newRow("8bit") << QStringLiteral("pdf417_8bit.png");
        QTest::newRow("24bit") << QStringLiteral("pdf417_24bit.png");
        QTest::newRow("32bit") << QStringLiteral("pdf417_32bit.png");
        QTest::newRow("cropped") << QStringLiteral("pdf417_cropped.png");
        QTest::newRow("rot90") << QStringLiteral("pdf417_rot90.png");
        QTest::newRow("rot180") << QStringLiteral("pdf417_rot180.png");
        QTest::newRow("rot270") << QStringLiteral("pdf417_rot270.png");
        QTest::newRow("flipped") << QStringLiteral("pdf417_flipped.png");
    }

    void testPDF417()
    {
        QFETCH(QString, fileName);

        QImage img(QStringLiteral(SOURCE_DIR "/barcodes/") + fileName);
        QVERIFY(!img.isNull());

#ifdef HAVE_ZXING
        QCOMPARE(BarcodeDecoder::decodePdf417(img), QStringLiteral("PDF417 is a stacked linear barcode symbol format used in a variety of applications, primarily transport, identification cards, and inventory management."));
#endif
    }

    void testAztec()
    {
        QImage img(QStringLiteral(SOURCE_DIR "/barcodes/aztec.png"));
        QVERIFY(!img.isNull());
#ifdef HAVE_ZXING
        QCOMPARE(BarcodeDecoder::decodeAztec(img), QStringLiteral("This is an example Aztec symbol for Wikipedia."));
#endif

        img.load(QStringLiteral(SOURCE_DIR "/barcodes/uic918-3star.png"));
        QVERIFY(!img.isNull());
#ifdef HAVE_ZXING
        const auto b = BarcodeDecoder::decodeAztecBinary(img);
        QCOMPARE(b.size(), 351);
        QVERIFY(b.startsWith("OTI010080000020"));
#endif
    }

    void testQRCode_data()
    {
            QTest::addColumn<QString>("fileName");
            QTest::addColumn<QString>("result");

            QTest::newRow("1") << QStringLiteral("qrcode1.png") << QStringLiteral("M$K0YGV0G");
            QTest::newRow("2") << QStringLiteral("qrcode2.png") << QStringLiteral("KZEXO4HRE");
    }

    void testQRCode()
    {
        QFETCH(QString, fileName);
        QFETCH(QString, result);

        QImage img(QStringLiteral(SOURCE_DIR "/barcodes/") + fileName);
        QVERIFY(!img.isNull());

#ifdef HAVE_ZXING
        QCOMPARE(BarcodeDecoder::decodeQRCode(img), result);
#endif

    }
};

QTEST_APPLESS_MAIN(BarcodeDecoderTest)

#include "barcodedecodertest.moc"
