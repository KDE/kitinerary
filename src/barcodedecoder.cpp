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
#ifdef HAVE_ZXING
    return decodeString(img, BarcodeFormat::AZTEC);
#else
    Q_UNUSED(img);
    return {};
#endif
}

QByteArray BarcodeDecoder::decodeAztecBinary(const QImage &img)
{
#ifdef HAVE_ZXING
    return decodeBinary(img, BarcodeFormat::AZTEC);
#else
    Q_UNUSED(img);
#endif
    return {};
}

QString BarcodeDecoder::decodeQRCode(const QImage &img)
{
#ifdef HAVE_ZXING
    return decodeString(img, BarcodeFormat::QR_CODE);
#else
    Q_UNUSED(img);
    return {};
#endif
}
