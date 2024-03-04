/*
    SPDX-FileCopyrightText: 2018-2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "config-kitinerary.h"

#include "barcodedecoder.h"
#include "logging.h"

#include <QDebug>
#include <QImage>
#include <QString>

#define ZX_USE_UTF8 1
#include <ZXing/ReadBarcode.h>

using namespace KItinerary;

enum {
    // unit is pixels, assuming landscape orientation
    MinSourceImageHeight = 10,
    MinSourceImageWidth = 26,
    // OEBB uses 1044x1044^W 2179x2179 for its UIC 918.3 Aztec code
    MaxSourceImageHeight = 2200,
    MaxSourceImageWidth = 2200
};


static constexpr const auto SQUARE_MAX_ASPECT = 1.25f;
static constexpr const auto PDF417_MIN_ASPECT = 1.5f;
static constexpr const auto PDF417_MAX_ASPECT = 6.5f;
static constexpr const auto ANY1D_MIN_ASPECT = 1.95f;
static constexpr const auto ANY1D_MAX_ASPECT = 8.0f;


QByteArray BarcodeDecoder::Result::toByteArray() const
{
    return (contentType & Result::ByteArray) ? content.toByteArray() : QByteArray();
}

QString BarcodeDecoder::Result::toString() const
{
    return (contentType & Result::String) ? content.toString() : QString();
}


BarcodeDecoder::BarcodeDecoder() = default;
BarcodeDecoder::~BarcodeDecoder() = default;

BarcodeDecoder::Result BarcodeDecoder::decode(const QImage &img, BarcodeDecoder::BarcodeTypes hint) const
{
    if ((hint & Any) == None || img.isNull()) {
        return {};
    }

    auto &results = m_cache[img.cacheKey()];
    if (results.size() > 1) {
        return Result{};
    }
    if (results.empty()) {
        results.push_back(Result{});
    }
    auto &result = results.front();
    decodeIfNeeded(img, hint, result);
    return (result.positive & hint) ? result : Result{};
}

std::vector<BarcodeDecoder::Result> BarcodeDecoder::decodeMulti(const QImage &img, BarcodeDecoder::BarcodeTypes hint) const
{
    if ((hint & Any) == None || img.isNull()) {
        return {};
    }

    auto &results = m_cache[img.cacheKey()];
    decodeMultiIfNeeded(img, hint, results);
    return (results.size() == 1 && (results[0].positive & hint) == 0) ? std::vector<Result>{} : results;
}

QByteArray BarcodeDecoder::decodeBinary(const QImage &img, BarcodeDecoder::BarcodeTypes hint) const
{
    return decode(img, hint).toByteArray();
}

QString BarcodeDecoder::decodeString(const QImage &img, BarcodeDecoder::BarcodeTypes hint) const
{
    return decode(img, hint).toString();
}

void BarcodeDecoder::clearCache()
{
    m_cache.clear();
}

BarcodeDecoder::BarcodeTypes BarcodeDecoder::isPlausibleSize(int width, int height, BarcodeDecoder::BarcodeTypes hint)
{
    // normalize to landscape
    if (height > width) {
        std::swap(width, height);
    }

    if (width > MinSourceImageWidth && height > MinSourceImageHeight
        && ((width < MaxSourceImageWidth && height < MaxSourceImageHeight) || (hint & IgnoreAspectRatio))) {
        return hint;
    }
    return None;
}

BarcodeDecoder::BarcodeTypes BarcodeDecoder::isPlausibleAspectRatio(int width, int height, BarcodeDecoder::BarcodeTypes hint)
{
    if (hint & IgnoreAspectRatio) {
        return hint;
    }

    // normalize to landscape
    if (height > width) {
        std::swap(width, height);
    }

    const auto aspectRatio = (float)width / (float)height;

    // almost square, assume Aztec or QR
    if (aspectRatio > SQUARE_MAX_ASPECT) {
        hint &= ~AnySquare;
    }

    // rectangular with medium aspect ratio, such as PDF 417
    if (aspectRatio < PDF417_MIN_ASPECT || aspectRatio > PDF417_MAX_ASPECT) {
        hint &= ~PDF417;
    }

    // 1D
    if (aspectRatio < ANY1D_MIN_ASPECT || aspectRatio > ANY1D_MAX_ASPECT) {
        hint &= ~Any1D;
    }

    return hint;
}

BarcodeDecoder::BarcodeTypes BarcodeDecoder::maybeBarcode(int width, int height, BarcodeDecoder::BarcodeTypes hint)
{
    return isPlausibleSize(width, height, hint) & isPlausibleAspectRatio(width, height, hint);
}

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
    ZXing::BarcodeFormats formats;

    for (auto i : zxing_format_map) {
        if (types & i.type) {
            formats |= i.zxingType;
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

static ZXing::ImageView zxingImageView(const QImage &img)
{
    return ZXing::ImageView{img.bits(), img.width(), img.height(), zxingImageFormat(img.format()), static_cast<int>(img.bytesPerLine())};
}

static void applyZXingResult(BarcodeDecoder::Result &result, const ZXing::Result &zxingResult, BarcodeDecoder::BarcodeTypes format)
{
    if (zxingResult.isValid()) {
#if ZXING_VERSION >= QT_VERSION_CHECK(1, 4, 0)
        // detect content type
        std::string zxUtf8Text;
        if (zxingResult.contentType() == ZXing::ContentType::Text) {
            result.contentType = BarcodeDecoder::Result::Any;
            zxUtf8Text = zxingResult.text();
            // check if the text is ASCII-only (in which case we allow access as byte array as well)
            if (std::any_of(zxUtf8Text.begin(), zxUtf8Text.end(), [](unsigned char c) { return c > 0x7F; })) {
                result.contentType &= ~BarcodeDecoder::Result::ByteArray;
            }
        } else {
            result.contentType = BarcodeDecoder::Result::ByteArray;
        }

        // decode content
        if (result.contentType & BarcodeDecoder::Result::ByteArray) {
            QByteArray b;
            b.resize(zxingResult.bytes().size());
            std::copy(zxingResult.bytes().begin(), zxingResult.bytes().end(), b.begin());
            result.content = b;
        } else {
            result.content = QString::fromStdString(zxUtf8Text);
        }
#else
        // detect content type
        result.contentType = BarcodeDecoder::Result::Any;
        if (std::any_of(zxingResult.text().begin(), zxingResult.text().end(), [](const auto c) { return c > 255; })) {
            result.contentType &= ~BarcodeDecoder::Result::ByteArray;
        }
        if (std::any_of(zxingResult.text().begin(), zxingResult.text().end(), [](const auto c) { return c < 0x20; })) {
            result.contentType &= ~BarcodeDecoder::Result::String;
        }

        // decode content
        if (result.contentType & BarcodeDecoder::Result::ByteArray) {
            QByteArray b;
            b.resize(zxingResult.text().size());
            std::copy(zxingResult.text().begin(), zxingResult.text().end(), b.begin());
            result.content = b;
        } else {
            result.content = QString::fromStdWString(zxingResult.text());
        }
#endif
        result.positive |= formatToType(zxingResult.format());
    } else {
        result.negative |= format;
    }
}

void BarcodeDecoder::decodeIfNeeded(const QImage &img, BarcodeDecoder::BarcodeTypes hint, BarcodeDecoder::Result &result) const
{
    if ((result.positive & hint) || (result.negative & hint) == hint) {
        return;
    }

    ZXing::DecodeHints hints;
    hints.setFormats(typeToFormats(hint));
    hints.setBinarizer(ZXing::Binarizer::FixedThreshold);
    hints.setIsPure((hint & BarcodeDecoder::IgnoreAspectRatio) == 0);

    // convert if img is in a format ZXing can't handle directly
#if ZXING_VERSION > QT_VERSION_CHECK(1, 3, 0)
    ZXing::Result res;
#else
    ZXing::Result res(ZXing::DecodeStatus::NotFound);
#endif
    if (zxingImageFormat(img.format()) == ZXing::ImageFormat::None) {
        res = ZXing::ReadBarcode(zxingImageView(img.convertToFormat(QImage::Format_Grayscale8)), hints);
    } else {
        res = ZXing::ReadBarcode(zxingImageView(img), hints);
    }

    applyZXingResult(result, res, hint);
}

void BarcodeDecoder::decodeMultiIfNeeded(const QImage &img, BarcodeDecoder::BarcodeTypes hint, std::vector<BarcodeDecoder::Result> &results) const
{
#if ZXING_VERSION > QT_VERSION_CHECK(1, 2, 0)
    if (std::any_of(results.begin(), results.end(), [hint](const auto &r) { return (r.positive & hint) || ((r.negative & hint) == hint); })) {
        return;
    }

    ZXing::DecodeHints hints;
    hints.setFormats(typeToFormats(hint));
    hints.setBinarizer(ZXing::Binarizer::FixedThreshold);
    hints.setIsPure(false);

    // convert if img is in a format ZXing can't handle directly
    std::vector<ZXing::Result> zxingResults;
    if (zxingImageFormat(img.format()) == ZXing::ImageFormat::None) {
        zxingResults = ZXing::ReadBarcodes(zxingImageView(img.convertToFormat(QImage::Format_Grayscale8)), hints);
    } else {
        zxingResults = ZXing::ReadBarcodes(zxingImageView(img), hints);
    }

    if (zxingResults.empty()) {
        Result r;
        r.negative |= hint;
        results.push_back(std::move(r));
    } else {
        // ### in theory we need to handle the case that we already have results from a previous run with different hints here...
        results.reserve(zxingResults.size());
        for (const auto &zxingRes : zxingResults) {
            Result r;
            applyZXingResult(r, zxingRes, hint);
            results.push_back(std::move(r));
        }
    }
#else
    // ZXing 1.2 has no multi-decode support yet, so always treat this as no hit
    Result r;
    r.negative |= hint;
    results.push_back(std::move(r));
#endif
}
