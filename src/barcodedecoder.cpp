/*
    Copyright (C) 2018-2019 Volker Krause <vkrause@kde.org>

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

#include "config-kitinerary.h"

#include "barcodedecoder.h"
#include "logging.h"
#include "qimagepurebinarizer.h"

#include <QDebug>
#include <QImage>
#include <QString>

#ifdef HAVE_ZXING
#include <ZXing/DecodeHints.h>
#include <ZXing/MultiFormatReader.h>
#include <ZXing/Result.h>
#endif

using namespace KItinerary;

enum {
    // unit is pixels, assuming landscape orientation
    MinSourceImageHeight = 10,
    MinSourceImageWidth = 30,
    // OEBB uses 1044x1044 for its UIC 918.3 Aztec code
    MaxSourceImageHeight = 1100, // TODO what's a realistic value here?
    MaxSourceImageWidth = 2000
};

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

bool BarcodeDecoder::isPlausibleSize(int width, int height)
{
    // normalize to landscape
    if (height > width) {
        std::swap(width, height);
    }

    return width > MinSourceImageWidth && height > MinSourceImageHeight && width < MaxSourceImageWidth && height < MaxSourceImageHeight;
}

bool BarcodeDecoder::isPlausibleAspectRatio(int width, int height, BarcodeDecoder::BarcodeTypes hint)
{
    // normalize to landscape
    if (height > width) {
        std::swap(width, height);
    }

    const auto aspectRatio = (float)width / (float)height;

    // almost square, assume Aztec or QR
    if (aspectRatio < 1.2f && (hint & AnySquare)) {
        return true;
    }

    // rectangular with medium aspect ratio, assume PDF 417
    return aspectRatio > 1.5 && aspectRatio < 6 && (hint  & PDF417);
}

bool BarcodeDecoder::maybeBarcode(int width, int height, BarcodeDecoder::BarcodeTypes hint)
{
    return isPlausibleSize(width, height) && isPlausibleAspectRatio(width, height, hint);
}

#ifdef HAVE_ZXING
static std::vector<ZXing::BarcodeFormat> typeToFormats(BarcodeDecoder::BarcodeTypes types)
{
    std::vector<ZXing::BarcodeFormat> formats;
    if (types & BarcodeDecoder::Aztec) {
        formats.push_back(ZXing::BarcodeFormat::AZTEC);
    }
    if (types & BarcodeDecoder::QRCode) {
        formats.push_back(ZXing::BarcodeFormat::QR_CODE);
    }
    if (types & BarcodeDecoder::PDF417) {
        formats.push_back(ZXing::BarcodeFormat::PDF_417);
    }
    return formats;
}

BarcodeDecoder::BarcodeType formatToType(ZXing::BarcodeFormat format)
{
    switch (format) {
        case ZXing::BarcodeFormat::AZTEC:
            return BarcodeDecoder::Aztec;
        case ZXing::BarcodeFormat::QR_CODE:
            return BarcodeDecoder::QRCode;
        case ZXing::BarcodeFormat::PDF_417:
            return BarcodeDecoder::PDF417;
        default:
            break;
    }
    return BarcodeDecoder::None;
}

void BarcodeDecoder::decodeZxing(const QImage &img, BarcodeDecoder::BarcodeTypes format, BarcodeDecoder::Result &result) const
{
    QImagePureBinarizer binarizer(img);
    ZXing::DecodeHints hints;
    hints.setPossibleFormats(typeToFormats(format));
    ZXing::MultiFormatReader reader(hints);
    const auto res = reader.read(binarizer);
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

    const auto aspectRatio = img.width() < img.height() ?
        (float)img.height() / (float)img.width() :
        (float)img.width() / (float)img.height();

    if (aspectRatio < 1.2f && (hint & AnySquare) && (result.negative & hint & AnySquare) != (hint & AnySquare)) {
        decodeZxing(img, hint & AnySquare, result);
    }

    if (aspectRatio > 1.5 && aspectRatio < 6 && (hint  & PDF417) && (result.negative & hint & PDF417) != (hint & PDF417)) {
        auto normalizedImg = img;
        if (normalizedImg.width() < normalizedImg.height()) {
            QTransform tf;
            tf.rotate(-90);
            normalizedImg = normalizedImg.transformed(tf);
        }

        decodeZxing(normalizedImg, PDF417, result);
        if (result.positive & PDF417) {
            return;
        }
        // try flipped around the x axis, zxing doesn't detect that, but it's e.g. encountered in SAS passes
        result.negative &= ~PDF417;
        decodeZxing(normalizedImg.transformed(QTransform{1, 0, 0, -1, 0, 0}), PDF417, result);
    }
}
