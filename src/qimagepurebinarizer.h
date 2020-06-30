/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_QIMAGEPUREBINARIZER_H
#define KITINERARY_QIMAGEPUREBINARIZER_H

#include "config-kitinerary.h"
#ifdef HAVE_ZXING

#include <QImage>

#include <ZXing/BinaryBitmap.h>
#include <ZXing/BitMatrix.h>

namespace KItinerary {

/** Binarizer of pure black/white barcode images.
 *  This bypasses the usually applied LuminanceSource and HybridBinarizer steps
 *  for input data from PDFs that don't need this.
 */
class QImagePureBinarizer : public ZXing::BinaryBitmap
{
public:
    explicit QImagePureBinarizer(const QImage &img);
    ~QImagePureBinarizer();

    int height() const override;
    int width() const override;
    bool isPureBarcode() const override;

    bool getBlackRow(int y, ZXing::BitArray &row) const override;
    std::shared_ptr<const ZXing::BitMatrix> getBlackMatrix() const override;

    bool canCrop() const override;
    bool canRotate() const override;
    std::shared_ptr<ZXing::BinaryBitmap> cropped(int left, int top, int width, int height) const override;
    std::shared_ptr<ZXing::BinaryBitmap> rotated(int degreeCW) const override;

private:
    QImage m_img;
    mutable std::shared_ptr<const ZXing::BitMatrix> m_bitmap;
};

}

#endif

#endif // KITINERARY_QIMAGEPUREBINARIZER_H
