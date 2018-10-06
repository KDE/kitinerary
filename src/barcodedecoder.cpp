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
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "config-kitinerary.h"

#include "barcodedecoder.h"
#include "logging.h"
#include "qimageluminancesource.h"
#include "qimagepurebinarizer.h"

#include <QDebug>
#include <QImage>
#include <QString>

#ifdef HAVE_ZXING
#include <ZXing/DecodeHints.h>
#include <ZXing/MultiFormatReader.h>
#include <ZXing/Result.h>
#elif defined(HAVE_ZXING_OLD)
#include <zxing/BinaryBitmap.h>
#include <zxing/DecodeHints.h>
#include <zxing/MultiFormatReader.h>
#include <zxing/Result.h>
#include <zxing/common/HybridBinarizer.h>
#endif

#include <exception>

using namespace KItinerary;

#ifdef HAVE_ZXING
using ZXing::BarcodeFormat;

static QString decodeString(const QImage &img, ZXing::BarcodeFormat format)
{
    QImagePureBinarizer binarizer(img);
    ZXing::DecodeHints hints;
    hints.setPossibleFormats({format});
    ZXing::MultiFormatReader reader(hints);
    const auto result = reader.read(binarizer);
    if (result.isValid()) {
        return QString::fromStdWString(result.text());
    }
    return {};
}

static QByteArray decodeBinary(const QImage &img, ZXing::BarcodeFormat format)
{
    QImagePureBinarizer binarizer(img);
    ZXing::DecodeHints hints;
    hints.setPossibleFormats({format});
    ZXing::MultiFormatReader reader(hints);
    auto result = reader.read(binarizer);
    if (result.isValid()) {
        QByteArray b;
        b.resize(result.text().size());
        std::copy(result.text().begin(), result.text().end(), b.begin());
        return b;
    }
    return  {};
}

#elif defined(HAVE_ZXING_OLD)
using zxing::BarcodeFormat;

static QString decodeString(const QImage &img, zxing::BarcodeFormat format)
{
    try {
        const zxing::Ref<zxing::LuminanceSource> source(new QImageLuminanceSource(img));
        const zxing::Ref<zxing::Binarizer> binarizer(new zxing::HybridBinarizer(source));
        const zxing::Ref<zxing::BinaryBitmap> binary(new zxing::BinaryBitmap(binarizer));
        const zxing::DecodeHints hints(1 << format);

        zxing::MultiFormatReader reader;
        const auto result = reader.decode(binary, hints);
        return QString::fromStdString(result->getText()->getText());
    } catch (const std::exception &e) {
        //qCDebug(Log) << e.what();
    }
    return {};
}

static QByteArray decodeBinary(const QImage &img, zxing::BarcodeFormat format)
{
    try {
        const zxing::Ref<zxing::LuminanceSource> source(new QImageLuminanceSource(img));
        const zxing::Ref<zxing::Binarizer> binarizer(new zxing::HybridBinarizer(source));
        const zxing::Ref<zxing::BinaryBitmap> binary(new zxing::BinaryBitmap(binarizer));
        const zxing::DecodeHints hints(1 << format);

        zxing::MultiFormatReader reader;
        const auto result = reader.decode(binary, hints);
        return QByteArray(result->getText()->getText().c_str(), result->getText()->getText().size());
    } catch (const std::exception &e) {
        //qCDebug(Log) << e.what();
    }
    return {};
}
#endif

QString BarcodeDecoder::decodePdf417(const QImage &img)
{
#ifdef HAVE_ZXING_ANY
    auto normalizedImg = img;
    if (normalizedImg.width() < normalizedImg.height()) {
        QTransform tf;
        tf.rotate(-90);
        normalizedImg = normalizedImg.transformed(tf);
    }

    const auto result = decodeString(normalizedImg, BarcodeFormat::PDF_417);
    if (!result.isEmpty()) {
        return result;
    }
    // try flipped around the x axis, zxing doesn't detect that, but it's e.g. encountered in SAS passes
    return decodeString(normalizedImg.transformed(QTransform{1, 0, 0, -1, 0, 0}), BarcodeFormat::PDF_417);
#else
    Q_UNUSED(img);
    return {};
#endif
}

QString BarcodeDecoder::decodeAztec(const QImage &img)
{
#ifdef HAVE_ZXING_ANY
    return decodeString(img, BarcodeFormat::AZTEC);
#else
    Q_UNUSED(img);
    return {};
#endif
}

QByteArray BarcodeDecoder::decodeAztecBinary(const QImage &img)
{
#ifdef HAVE_ZXING_ANY
    return decodeBinary(img, BarcodeFormat::AZTEC);
#else
    Q_UNUSED(img);
#endif
    return {};
}

QString BarcodeDecoder::decodeQRCode(const QImage &img)
{
#ifdef HAVE_ZXING_ANY
    return decodeString(img, BarcodeFormat::QR_CODE);
#else
    Q_UNUSED(img);
    return {};
#endif
}
