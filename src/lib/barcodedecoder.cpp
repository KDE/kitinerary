/*
    SPDX-FileCopyrightText: 2018-2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "config-kitinerary.h"

#include "barcodedecoder.h"
#include "logging.h"
#include "qimagepurebinarizer.h"

#include <QDebug>
#include <QImage>
#include <QString>

#ifdef HAVE_ZXING
#ifdef ZXING_USE_READBARCODE
#include <ZXing/ReadBarcode.h>
#else
#include <ZXing/DecodeHints.h>
#include <ZXing/MultiFormatReader.h>
#include <ZXing/Result.h>
#endif
#endif

using namespace KItinerary;

enum {
    // unit is pixels, assuming landscape orientation
    MinSourceImageHeight = 10,
    MinSourceImageWidth = 26,
    // OEBB uses 1044x1044 for its UIC 918.3 Aztec code
    MaxSourceImageHeight = 1100, // TODO what's a realistic value here?
    MaxSourceImageWidth = 2000
};


static constexpr const auto SQUARE_MAX_ASPECT = 1.25f;
static constexpr const auto PDF417_MIN_ASPECT = 1.5f;
static constexpr const auto PDF417_MAX_ASPECT = 6.0f;
static constexpr const auto ANY1D_MIN_ASPECT = 6.0f;

BarcodeDecoder::BarcodeDecoder() = default;
BarcodeDecoder::~BarcodeDecoder() = default;

bool BarcodeDecoder::isBarcode(const QImage &img, BarcodeDecoder::BarcodeTypes hint) const
{
    if (!maybeBarcode(img.width(), img.height(), hint)) {
        return false;
    }

    auto &result = m_cache[img.cacheKey()];
    decodeIfNeeded(img, hint, result);
    return hint & result.positive;
}

QByteArray BarcodeDecoder::decodeBinary(const QImage &img, BarcodeDecoder::BarcodeTypes hint) const
{
    if (!maybeBarcode(img.width(), img.height(), hint)) {
        return {};
    }

    auto &result = m_cache[img.cacheKey()];
    decodeIfNeeded(img, hint, result);
    if ((result.positive & hint) && (result.contentType & Result::ByteArray)) {
        return result.content.toByteArray();
    }

    return {};
}

QString BarcodeDecoder::decodeString(const QImage &img, BarcodeDecoder::BarcodeTypes hint) const
{
    if (!maybeBarcode(img.width(), img.height(), hint)) {
        return {};
    }

    auto &result = m_cache[img.cacheKey()];
    decodeIfNeeded(img, hint, result);
    if ((result.positive & hint) && (result.contentType & Result::String)) {
        return result.content.toString();
    }

    return {};
}

void BarcodeDecoder::clearCache()
{
    m_cache.clear();
}

bool BarcodeDecoder::isPlausibleSize(int width, int height, BarcodeDecoder::BarcodeTypes hint)
{
    // normalize to landscape
    if (height > width) {
        std::swap(width, height);
    }

    return width > MinSourceImageWidth && height > MinSourceImageHeight
        && ((width < MaxSourceImageWidth && height < MaxSourceImageHeight) || (hint & IgnoreAspectRatio));
}

bool BarcodeDecoder::isPlausibleAspectRatio(int width, int height, BarcodeDecoder::BarcodeTypes hint)
{
    if (hint & IgnoreAspectRatio) {
        return true;
    }

    // normalize to landscape
    if (height > width) {
        std::swap(width, height);
    }

    const auto aspectRatio = (float)width / (float)height;

    // almost square, assume Aztec or QR
    if (aspectRatio < SQUARE_MAX_ASPECT && (hint & AnySquare)) {
        return true;
    }

    // rectangular with medium aspect ratio, such as PDF 417
    if (aspectRatio > PDF417_MIN_ASPECT && aspectRatio < PDF417_MAX_ASPECT && (hint & PDF417)) {
        return true;
    }

    // 1D
    if (aspectRatio > ANY1D_MIN_ASPECT && (hint & Any1D)) {
        return true;
    }

    return false;
}

bool BarcodeDecoder::maybeBarcode(int width, int height, BarcodeDecoder::BarcodeTypes hint)
{
    return isPlausibleSize(width, height, hint) && isPlausibleAspectRatio(width, height, hint);
}

#ifdef HAVE_ZXING
struct {
    BarcodeDecoder::BarcodeType type;
    ZXing::BarcodeFormat zxingType;
} static constexpr const zxing_format_map[] = {
#if ZXING_VERSION > QT_VERSION_CHECK(1, 1, 1)
    { BarcodeDecoder::Aztec, ZXing::BarcodeFormat::Aztec },
    { BarcodeDecoder::QRCode, ZXing::BarcodeFormat::QRCode },
    { BarcodeDecoder::PDF417, ZXing::BarcodeFormat::PDF417 },
    { BarcodeDecoder::DataMatrix, ZXing::BarcodeFormat::DataMatrix },
    { BarcodeDecoder::Code39, ZXing::BarcodeFormat::Code39 },
    { BarcodeDecoder::Code93, ZXing::BarcodeFormat::Code93 },
    { BarcodeDecoder::Code128, ZXing::BarcodeFormat::Code128 },
#else
    { BarcodeDecoder::Aztec, ZXing::BarcodeFormat::AZTEC },
    { BarcodeDecoder::QRCode, ZXing::BarcodeFormat::QR_CODE },
    { BarcodeDecoder::PDF417, ZXing::BarcodeFormat::PDF_417 },
    { BarcodeDecoder::DataMatrix, ZXing::BarcodeFormat::DATA_MATRIX },
    { BarcodeDecoder::Code39, ZXing::BarcodeFormat::CODE_39 },
    { BarcodeDecoder::Code93, ZXing::BarcodeFormat::CODE_93 },
    { BarcodeDecoder::Code128, ZXing::BarcodeFormat::CODE_128 },
#endif
};

static auto typeToFormats(BarcodeDecoder::BarcodeTypes types)
{
#if ZXING_VERSION >= QT_VERSION_CHECK(1, 1, 0)
    ZXing::BarcodeFormats formats;
#else
    std::vector<ZXing::BarcodeFormat> formats;
#endif

    for (auto i : zxing_format_map) {
        if (types & i.type) {
#if ZXING_VERSION >= QT_VERSION_CHECK(1, 1, 0)
            formats |= i.zxingType;
#else
            formats.push_back(i.zxingType);
#endif
        }
    }
    return formats;
}

BarcodeDecoder::BarcodeType formatToType(ZXing::BarcodeFormat format)
{
    for (auto i : zxing_format_map) {
        if (format == i.zxingType) {
            return i.type;
        }
    }
    return BarcodeDecoder::None;
}

#ifdef ZXING_USE_READBARCODE
static ZXing::ImageFormat zxingImageFormat(QImage::Format format)
{
    switch (format) {
        case QImage::Format_ARGB32:
        case QImage::Format_RGB32:
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
            return ZXing::ImageFormat::BGRX;
#else
            return ZXing::ImageFormat::XRGB;
#endif
        case QImage::Format_RGB888:
            return ZXing::ImageFormat::RGB;
        case QImage::Format_RGBX8888:
        case QImage::Format_RGBA8888:
            return ZXing::ImageFormat::RGBX;
        case QImage::Format_Grayscale8:
            return ZXing::ImageFormat::Lum;
        default:
            return ZXing::ImageFormat::None;
    }
    Q_UNREACHABLE();
}

static ZXing::Result zxingReadBarcode(const QImage &img, const ZXing::DecodeHints &hints)
{
    return ZXing::ReadBarcode({img.bits(), img.width(), img.height(), zxingImageFormat(img.format()), static_cast<int>(img.bytesPerLine())}, hints);
}
#endif

void BarcodeDecoder::decodeZxing(const QImage &img, BarcodeDecoder::BarcodeTypes format, BarcodeDecoder::Result &result) const
{
    ZXing::DecodeHints hints;
#if ZXING_VERSION >= QT_VERSION_CHECK(1, 1, 0)
    hints.setFormats(typeToFormats(format));
#else
    hints.setPossibleFormats(typeToFormats(format));
#endif

#ifdef ZXING_USE_READBARCODE
    hints.setBinarizer(ZXing::Binarizer::FixedThreshold);
    hints.setIsPure((format & BarcodeDecoder::IgnoreAspectRatio) == 0);

    // convert if img is in a format ZXing can't handle directly
    const auto res = zxingImageFormat(img.format()) == ZXing::ImageFormat::None ?
        zxingReadBarcode(img.convertToFormat(QImage::Format_Grayscale8), hints) : zxingReadBarcode(img, hints);
#else
    QImagePureBinarizer binarizer(img);
    ZXing::MultiFormatReader reader(hints);
    const auto res = reader.read(binarizer);
#endif

    if (res.isValid()) {
        // detect content type
        result.contentType = Result::Any;
        if (std::any_of(res.text().begin(), res.text().end(), [](const auto c) { return c > 255; })) {
            result.contentType &= ~Result::ByteArray;
        }
        if (std::any_of(res.text().begin(), res.text().end(), [](const auto c) { return c < 20; })) {
            result.contentType &= ~Result::String;
        }

        // decode content
        if (result.contentType & Result::ByteArray) {
            QByteArray b;
            b.resize(res.text().size());
            std::copy(res.text().begin(), res.text().end(), b.begin());
            result.content = b;
        } else {
            result.content = QString::fromStdWString(res.text());
        }
        result.positive |= formatToType(res.format());
    } else {
        result.negative |= format;
    }
}
#else
void BarcodeDecoder::decodeZxing(const QImage&, BarcodeDecoder::BarcodeTypes, BarcodeDecoder::Result&) const {}
#endif

void BarcodeDecoder::decodeIfNeeded(const QImage &img, BarcodeDecoder::BarcodeTypes hint, BarcodeDecoder::Result &result) const
{
    if ((result.positive & hint) || (result.negative & hint) == hint) {
        return;
    }

    if (hint & IgnoreAspectRatio) {
        decodeZxing(img, hint, result);
        return;
    }

    const auto aspectRatio = img.width() < img.height() ?
        (float)img.height() / (float)img.width() :
        (float)img.width() / (float)img.height();

    if (aspectRatio < SQUARE_MAX_ASPECT && (hint & AnySquare) && (result.negative & hint & AnySquare) != (hint & AnySquare)) {
        decodeZxing(img, hint & AnySquare, result);
    }

    if (aspectRatio > PDF417_MIN_ASPECT && aspectRatio < PDF417_MAX_ASPECT && (hint  & PDF417) && (result.negative & hint & PDF417) != (hint & PDF417)) {
        auto normalizedImg = img;
        // newer ZXing versions handle rotated/flipped codes themselves correctly
#ifndef ZXING_USE_READBARCODE
        if (normalizedImg.width() < normalizedImg.height()) {
            QTransform tf;
            tf.rotate(-90);
            normalizedImg = normalizedImg.transformed(tf);
        }
#endif

        decodeZxing(normalizedImg, PDF417, result);
#ifndef ZXING_USE_READBARCODE
        if (result.positive & PDF417) {
            return;
        }
        // try flipped around the x axis, zxing doesn't detect that, but it's e.g. encountered in SAS passes
        result.negative &= ~PDF417;
        decodeZxing(normalizedImg.transformed(QTransform{1, 0, 0, -1, 0, 0}), PDF417, result);
#endif
    }

    if (aspectRatio > ANY1D_MIN_ASPECT && (hint & Any1D) && (result.negative & hint & Any1D) != (hint & Any1D)) {
        decodeZxing(img, hint & Any1D, result);
    }
}
