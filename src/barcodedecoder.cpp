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

#include <QDebug>
#include <QImage>
#include <QString>

#ifdef HAVE_ZXING
#include <zxing/BinaryBitmap.h>
#include <zxing/DecodeHints.h>
#include <zxing/LuminanceSource.h>
#include <zxing/MultiFormatReader.h>
#include <zxing/Result.h>
#include <zxing/common/HybridBinarizer.h>
#endif

#include <exception>

using namespace KItinerary;

namespace KItinerary {
#ifdef HAVE_ZXING
class QImageLuminanceSource : public zxing::LuminanceSource
{
public:
    explicit QImageLuminanceSource(const QImage &img);

    zxing::ArrayRef<char> getRow(int y, zxing::ArrayRef<char> row) const override;
    zxing::ArrayRef<char> getMatrix() const override;

private:
    char luminance(int x, int y) const;

    QImage m_img;
};

#endif
}

#ifdef HAVE_ZXING
QImageLuminanceSource::QImageLuminanceSource(const QImage &img)
    : zxing::LuminanceSource(img.width(), img.height())
    , m_img(img)
{
}

zxing::ArrayRef<char> QImageLuminanceSource::getRow(int y, zxing::ArrayRef<char> row) const
{
    if (!row) {
        row = zxing::ArrayRef<char>(getWidth());
    }

    for (int i = 0; i < getWidth(); ++i) {
        row[i] = luminance(i, y);
    }

    return row;
}

zxing::ArrayRef<char> QImageLuminanceSource::getMatrix() const
{
    zxing::ArrayRef<char> matrix(getWidth() * getHeight());

    for (int i = 0; i < getHeight(); ++i) {
        for (int j = 0; j < getWidth(); ++j) {
            matrix[i * getWidth() + j] = luminance(j, i);
        }
    }

    return matrix;
}

char QImageLuminanceSource::luminance(int x, int y) const
{
    return qGray(m_img.pixel(x, y));
}

#endif


QString BarcodeDecoder::decodePdf417(const QImage &img)
{
#ifdef HAVE_ZXING
    try {
        const zxing::Ref<zxing::LuminanceSource> source(new QImageLuminanceSource(img));

        const zxing::Ref<zxing::Binarizer> binarizer(new zxing::HybridBinarizer(source));
        const zxing::Ref<zxing::BinaryBitmap> binary(new zxing::BinaryBitmap(binarizer));
        const zxing::DecodeHints hints(zxing::DecodeHints::PDF_417_HINT);

        zxing::MultiFormatReader reader;
        const auto result = reader.decode(binary, hints);
        return QString::fromStdString(result->getText()->getText());
    } catch (const std::exception &e) {
        qCWarning(Log) << e.what();
    }
#else
    Q_UNUSED(img);
#endif
    return {};
}

QString BarcodeDecoder::decodeAztec(const QImage &img)
{
    return QString::fromUtf8(decodeAztecBinary(img));
}

QByteArray BarcodeDecoder::decodeAztecBinary(const QImage &img)
{
#ifdef HAVE_ZXING
    try {
        const zxing::Ref<zxing::LuminanceSource> source(new QImageLuminanceSource(img));

        const zxing::Ref<zxing::Binarizer> binarizer(new zxing::HybridBinarizer(source));
        const zxing::Ref<zxing::BinaryBitmap> binary(new zxing::BinaryBitmap(binarizer));
        const zxing::DecodeHints hints(zxing::DecodeHints::AZTEC_HINT);

        zxing::MultiFormatReader reader;
        const auto result = reader.decode(binary, hints);
        return QByteArray(result->getText()->getText().c_str(), result->getText()->getText().size());
    } catch (const std::exception &e) {
        qCWarning(Log) << e.what();
    }
#else
    Q_UNUSED(img);
#endif
    return {};
}
