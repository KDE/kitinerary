/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <config-kitinerary.h>
#include <KItinerary/BarcodeDecoder>

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

        BarcodeDecoder decoder;
#ifdef HAVE_ZXING
        QCOMPARE(decoder.decodeString(img, BarcodeDecoder::PDF417), QStringLiteral("PDF417 is a stacked linear barcode symbol format used in a variety of applications, primarily transport, identification cards, and inventory management."));
#endif
    }

    void testAztec()
    {
        QImage img(QStringLiteral(SOURCE_DIR "/barcodes/aztec.png"));
        QVERIFY(!img.isNull());
        BarcodeDecoder decoder;
#ifdef HAVE_ZXING
        QCOMPARE(decoder.decodeString(img, BarcodeDecoder::Aztec), QStringLiteral("This is an example Aztec symbol for Wikipedia."));
#endif
        img.load(QStringLiteral(SOURCE_DIR "/barcodes/aztec-partial-quiet-zone.png"));
        QVERIFY(!img.isNull());
#ifdef HAVE_ZXING
        QCOMPARE(decoder.decodeString(img, BarcodeDecoder::Aztec), QStringLiteral("KF5::Prison - The KDE barcode generation framework."));
#endif

        img.load(QStringLiteral(SOURCE_DIR "/barcodes/uic918-3star.png"));
        QVERIFY(!img.isNull());
#ifdef HAVE_ZXING
        const auto b = decoder.decodeBinary(img, BarcodeDecoder::Aztec);
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

        BarcodeDecoder decoder;
#ifdef HAVE_ZXING
        QCOMPARE(decoder.decodeString(img, BarcodeDecoder::QRCode), result);
#endif

    }

    void testPlausibilityCheck()
    {
        QVERIFY(!BarcodeDecoder::maybeBarcode(10, 10));
        QVERIFY(BarcodeDecoder::maybeBarcode(100, 100));
        QVERIFY(!BarcodeDecoder::maybeBarcode(100, 100, BarcodeDecoder::PDF417));
        QVERIFY(BarcodeDecoder::maybeBarcode(100, 100, BarcodeDecoder::Aztec));
        QVERIFY(BarcodeDecoder::maybeBarcode(100, 100, BarcodeDecoder::AnySquare));
        QVERIFY(BarcodeDecoder::maybeBarcode(100, 180, BarcodeDecoder::PDF417));
        QVERIFY(BarcodeDecoder::maybeBarcode(180, 100, BarcodeDecoder::PDF417));
        QVERIFY(!BarcodeDecoder::maybeBarcode(180, 100, BarcodeDecoder::AnySquare));
    }

    void testNoCode()
    {
        BarcodeDecoder decoder;
        QVERIFY(!decoder.isBarcode({}));
        QCOMPARE(decoder.decodeBinary({}), QByteArray());
        QCOMPARE(decoder.decodeString({}), QString());

        QImage img(10, 10, QImage::Format_ARGB32);
        img.fill(Qt::black);
        QVERIFY(!decoder.isBarcode(img));
        QCOMPARE(decoder.decodeBinary(img), QByteArray());
        QCOMPARE(decoder.decodeString(img), QString());

        img = QImage(200, 200, QImage::Format_ARGB32);
        img.fill(Qt::red);
        img.fill(Qt::black);
        QVERIFY(!decoder.isBarcode(img));
        QCOMPARE(decoder.decodeBinary(img), QByteArray());
        QCOMPARE(decoder.decodeString(img), QString());
    }

    void testDecoderHints()
    {
        BarcodeDecoder decoder;
        QImage img(QStringLiteral(SOURCE_DIR "/barcodes/aztec.png"));
        QCOMPARE(decoder.decodeString(img, BarcodeDecoder::PDF417), QString());
#ifdef HAVE_ZXING
        QCOMPARE(decoder.decodeString(img, BarcodeDecoder::AnySquare), QStringLiteral("This is an example Aztec symbol for Wikipedia."));
        QCOMPARE(decoder.decodeString(img, BarcodeDecoder::Any), QStringLiteral("This is an example Aztec symbol for Wikipedia."));
        QCOMPARE(decoder.decodeString(img, BarcodeDecoder::Aztec), QStringLiteral("This is an example Aztec symbol for Wikipedia."));
        QCOMPARE(decoder.decodeString(img, BarcodeDecoder::QRCode), QString());

        decoder.clearCache();
        QVERIFY(decoder.isBarcode(img));
        QVERIFY(!decoder.isBarcode(img, BarcodeDecoder::PDF417));
        QVERIFY(!decoder.isBarcode(img, BarcodeDecoder::QRCode));
        QVERIFY(decoder.isBarcode(img, BarcodeDecoder::Aztec));
        QVERIFY(decoder.isBarcode(img, BarcodeDecoder::AnySquare));
#endif
    }

    void testContentTypeDetection()
    {
        BarcodeDecoder decoder;
        QImage img(QStringLiteral(SOURCE_DIR "/barcodes/aztec.png"));
#ifdef HAVE_ZXING
        QCOMPARE(decoder.decodeString(img, BarcodeDecoder::Any), QStringLiteral("This is an example Aztec symbol for Wikipedia."));
        QCOMPARE(decoder.decodeBinary(img, BarcodeDecoder::Any), QByteArray("This is an example Aztec symbol for Wikipedia."));

        img.load(QStringLiteral(SOURCE_DIR "/barcodes/uic918-3star.png"));
        QCOMPARE(decoder.decodeString(img, BarcodeDecoder::Any), QString());
        const auto ba = decoder.decodeBinary(img, BarcodeDecoder::Any);
        QCOMPARE(ba.size(), 351);
        QVERIFY(ba.startsWith("OTI010080000020"));
#endif
    }
};

QTEST_APPLESS_MAIN(BarcodeDecoderTest)

#include "barcodedecodertest.moc"
