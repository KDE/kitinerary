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

#include <QDebug>
#include <QImage>
#include <QString>

#ifdef HAVE_ZXING
#include <zxing/BinaryBitmap.h>
#include <zxing/DecodeHints.h>
#include <zxing/MultiFormatReader.h>
#include <zxing/Result.h>
#include <zxing/common/HybridBinarizer.h>
#endif

#include <exception>

using namespace KItinerary;

#ifdef HAVE_ZXING
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
#ifdef HAVE_ZXING
    auto normalizedImg = img;
    if (normalizedImg.width() < normalizedImg.height()) {
        QTransform tf;
        tf.rotate(-90);
        normalizedImg = normalizedImg.transformed(tf);
    }

    const auto result = decodeString(normalizedImg, zxing::BarcodeFormat::PDF_417);
    if (!result.isEmpty()) {
        return result;
    }
    // try flipped around the x axis, zxing doesn't detect that, but it's e.g. encountered in SAS passes
    return decodeString(normalizedImg.transformed(QTransform{1, 0, 0, -1, 0, 0}), zxing::BarcodeFormat::PDF_417);
#else
    Q_UNUSED(img);
    return {};
#endif
}

QString BarcodeDecoder::decodeAztec(const QImage &img)
{
#ifdef HAVE_ZXING
    return decodeString(img, zxing::BarcodeFormat::AZTEC);
#else
    Q_UNUSED(img);
    return {};
#endif
}

QByteArray BarcodeDecoder::decodeAztecBinary(const QImage &img)
{
#ifdef HAVE_ZXING
    return decodeBinary(img, zxing::BarcodeFormat::AZTEC);
#else
    Q_UNUSED(img);
#endif
    return {};
}

QString BarcodeDecoder::decodeQRCode(const QImage &img)
{
#ifdef HAVE_ZXING
    return decodeString(img, zxing::BarcodeFormat::QR_CODE);
#else
    Q_UNUSED(img);
    return {};
#endif
}
