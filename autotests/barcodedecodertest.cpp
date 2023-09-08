/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KItinerary/BarcodeDecoder>

#include <QImage>
#include <QObject>
#include <QTest>

Q_DECLARE_METATYPE(KItinerary::BarcodeDecoder::BarcodeType)

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
        QTest::newRow("partial cropped") << QStringLiteral("pdf417_partial_cropped.png");
    }

    void testPDF417()
    {
        QFETCH(QString, fileName);

        QImage img(QStringLiteral(SOURCE_DIR "/barcodes/") + fileName);
        QVERIFY(!img.isNull());

        BarcodeDecoder decoder;
        QVERIFY(BarcodeDecoder::maybeBarcode(img.width(), img.height(), BarcodeDecoder::Any) & BarcodeDecoder::PDF417);
        QCOMPARE(decoder.decode(img, BarcodeDecoder::PDF417).toString(), QStringLiteral("PDF417 is a stacked linear barcode symbol format used in a variety of applications, primarily transport, identification cards, and inventory management."));
    }

    void testAztec()
    {
        QImage img(QStringLiteral(SOURCE_DIR "/barcodes/aztec.png"));
        QVERIFY(!img.isNull());
        BarcodeDecoder decoder;
        QVERIFY(BarcodeDecoder::maybeBarcode(img.width(), img.height(), BarcodeDecoder::Any) & BarcodeDecoder::Aztec);
        QCOMPARE(decoder.decode(img, BarcodeDecoder::Aztec).toString(), QStringLiteral("This is an example Aztec symbol for Wikipedia."));
        img.load(QStringLiteral(SOURCE_DIR "/barcodes/aztec-partial-quiet-zone.png"));
        QVERIFY(!img.isNull());
        QVERIFY(BarcodeDecoder::maybeBarcode(img.width(), img.height(), BarcodeDecoder::Any) & BarcodeDecoder::Aztec);
        QCOMPARE(decoder.decode(img, BarcodeDecoder::Aztec).toString(), QStringLiteral("KF5::Prison - The KDE barcode generation framework."));

        img.load(QStringLiteral(SOURCE_DIR "/barcodes/uic918-3star.png"));
        QVERIFY(!img.isNull());
        QVERIFY(BarcodeDecoder::maybeBarcode(img.width(), img.height(), BarcodeDecoder::Any) & BarcodeDecoder::Aztec);
        const auto b = decoder.decode(img, BarcodeDecoder::Aztec).toByteArray();
        QCOMPARE(b.size(), 351);
        QVERIFY(b.startsWith("OTI010080000020"));
    }

    void testQRCode_data()
    {
            QTest::addColumn<QString>("fileName");
            QTest::addColumn<QString>("result");

            QTest::newRow("1") << QStringLiteral("qrcode1.png") << QStringLiteral("M$K0YGV0G");
            QTest::newRow("2") << QStringLiteral("qrcode2.png") << QStringLiteral("KZEXO4HRE");
            // individual lines have non-square features (found e.g. in Renfe tickets)
            QTest::newRow("3") << QStringLiteral("qrcode-scale-artifacts.png") << QStringLiteral("M$K0YGV0G");
    }

    void testQRCode()
    {
        QFETCH(QString, fileName);
        QFETCH(QString, result);

        QImage img(QStringLiteral(SOURCE_DIR "/barcodes/") + fileName);
        QVERIFY(!img.isNull());

        BarcodeDecoder decoder;
        QVERIFY(BarcodeDecoder::maybeBarcode(img.width(), img.height(), BarcodeDecoder::Any) & BarcodeDecoder::QRCode);
        QCOMPARE(decoder.decode(img, BarcodeDecoder::QRCode).toString(), result);
    }

    void testPlausibilityCheck()
    {
        QCOMPARE(BarcodeDecoder::maybeBarcode(10, 10, BarcodeDecoder::Any), BarcodeDecoder::None);
        QCOMPARE(BarcodeDecoder::maybeBarcode(100, 100, BarcodeDecoder::Any), BarcodeDecoder::AnySquare);
        QCOMPARE(BarcodeDecoder::maybeBarcode(100, 100, BarcodeDecoder::PDF417), BarcodeDecoder::None);
        QCOMPARE(BarcodeDecoder::maybeBarcode(100, 100, BarcodeDecoder::Aztec), BarcodeDecoder::Aztec);
        QCOMPARE(BarcodeDecoder::maybeBarcode(100, 100, BarcodeDecoder::AnySquare), BarcodeDecoder::AnySquare);
        QCOMPARE(BarcodeDecoder::maybeBarcode(100, 180, BarcodeDecoder::PDF417), BarcodeDecoder::PDF417);
        QCOMPARE(BarcodeDecoder::maybeBarcode(180, 100, BarcodeDecoder::PDF417), BarcodeDecoder::PDF417);
        QCOMPARE(BarcodeDecoder::maybeBarcode(180, 100, BarcodeDecoder::AnySquare), BarcodeDecoder::None);
    }

    void testNoCode()
    {
        BarcodeDecoder decoder;
        QCOMPARE(decoder.decode({}, BarcodeDecoder::Any).contentType, BarcodeDecoder::None);
        QCOMPARE(decoder.decode({}, BarcodeDecoder::Any).toByteArray(), QByteArray());
        QCOMPARE(decoder.decode({}, BarcodeDecoder::Any).toString(), QString());

        QImage img(10, 10, QImage::Format_ARGB32);
        img.fill(Qt::black);
        QCOMPARE(decoder.decode(img, BarcodeDecoder::Any).contentType, BarcodeDecoder::None);
        QCOMPARE(decoder.decode(img, BarcodeDecoder::Any).toByteArray(), QByteArray());
        QCOMPARE(decoder.decode(img, BarcodeDecoder::Any).toString(), QString());

        img = QImage(200, 200, QImage::Format_ARGB32);
        img.fill(Qt::red);
        img.fill(Qt::black);
        QCOMPARE(decoder.decode(img, BarcodeDecoder::Any).contentType, BarcodeDecoder::None);
        QCOMPARE(decoder.decode(img, BarcodeDecoder::Any).toByteArray(), QByteArray());
        QCOMPARE(decoder.decode(img, BarcodeDecoder::Any).toString(), QString());
    }

    void testDecoderHints()
    {
        BarcodeDecoder decoder;
        QImage img(QStringLiteral(SOURCE_DIR "/barcodes/aztec.png"));
        QCOMPARE(decoder.decode(img, BarcodeDecoder::PDF417).toString(), QString());
        QCOMPARE(decoder.decode(img, BarcodeDecoder::AnySquare).toString(), QStringLiteral("This is an example Aztec symbol for Wikipedia."));
        QCOMPARE(decoder.decode(img, BarcodeDecoder::Any).toString(), QStringLiteral("This is an example Aztec symbol for Wikipedia."));
        QCOMPARE(decoder.decode(img, BarcodeDecoder::Aztec).toString(), QStringLiteral("This is an example Aztec symbol for Wikipedia."));
        QCOMPARE(decoder.decode(img, BarcodeDecoder::QRCode).toString(), QString());

        decoder.clearCache();
        QCOMPARE(decoder.decode(img, BarcodeDecoder::Any).toString(), QStringLiteral("This is an example Aztec symbol for Wikipedia."));
        QCOMPARE(decoder.decode(img, BarcodeDecoder::PDF417).toString(), QString());
        QCOMPARE(decoder.decode(img, BarcodeDecoder::QRCode).toString(), QString());
        QCOMPARE(decoder.decode(img, BarcodeDecoder::Aztec).toString(), QStringLiteral("This is an example Aztec symbol for Wikipedia."));
        QCOMPARE(decoder.decode(img, BarcodeDecoder::AnySquare).toString(), QStringLiteral("This is an example Aztec symbol for Wikipedia."));
    }

    void testContentTypeDetection()
    {
        BarcodeDecoder decoder;
        QImage img(QStringLiteral(SOURCE_DIR "/barcodes/aztec.png"));
        QCOMPARE(decoder.decode(img, BarcodeDecoder::Any).toString(), QStringLiteral("This is an example Aztec symbol for Wikipedia."));
        QCOMPARE(decoder.decode(img, BarcodeDecoder::Any).toByteArray(), QByteArray("This is an example Aztec symbol for Wikipedia."));

        img.load(QStringLiteral(SOURCE_DIR "/barcodes/uic918-3star.png"));
        QCOMPARE(decoder.decode(img, BarcodeDecoder::Any).toString(), QString());
        const auto ba = decoder.decode(img, BarcodeDecoder::Any).toByteArray();
        QCOMPARE(ba.size(), 351);
        QVERIFY(ba.startsWith("OTI010080000020"));
    }

    void test1D_data()
    {
        QTest::addColumn<QString>("filename");
        QTest::addColumn<BarcodeDecoder::BarcodeType>("type");

        QTest::newRow("code39") << QStringLiteral("code39.png") << BarcodeDecoder::Code39;
        QTest::newRow("code93") << QStringLiteral("code93.png") << BarcodeDecoder::Code93;
        QTest::newRow("code128") << QStringLiteral("code128.png") << BarcodeDecoder::Code128;
    }

    void test1D()
    {
        QFETCH(QString, filename);
        QFETCH(BarcodeDecoder::BarcodeType, type);

        QImage img(QLatin1String(SOURCE_DIR "/barcodes/") + filename);
        QVERIFY(!img.isNull());

        BarcodeDecoder decoder;
        QCOMPARE(decoder.decode(img, BarcodeDecoder::Any2D).toString(), QString());
        QCOMPARE(decoder.decode(img, type).toString(), QLatin1String("123456789"));
        QCOMPARE(decoder.decode(img, BarcodeDecoder::Any1D).toString(), QLatin1String("123456789"));
        QCOMPARE(decoder.decode(img, BarcodeDecoder::Any).toString(), QLatin1String("123456789"));
    }
};

QTEST_APPLESS_MAIN(BarcodeDecoderTest)

#include "barcodedecodertest.moc"
