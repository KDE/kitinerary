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

#include "qimagepurebinarizer.h"
#ifdef HAVE_ZXING

#include <ZXing/BitArray.h>

using namespace KItinerary;

QImagePureBinarizer::QImagePureBinarizer(const QImage &img)
    : m_img(img)
{
}

QImagePureBinarizer::~QImagePureBinarizer() = default;

int QImagePureBinarizer::height() const
{
    return m_img.height() + 2;
}

int QImagePureBinarizer::width() const
{
    return m_img.width() + 2;
}

bool QImagePureBinarizer::isPureBarcode() const
{
    return true;
}

bool QImagePureBinarizer::getBlackRow(int y, ZXing::BitArray &row) const
{
    using namespace ZXing;

    const auto w = width();
    if (row.size() != w) {
        row = BitArray(w);
    } else {
        row.clearBits();
    }

    if (y == 0 || y == height() - 1) {
        return true;
    }

    for (auto i = 1; i < w - 1; ++i) {
        if (qGray(m_img.pixel(i - 1, y - 1)) < 127)
            row.set(i);
    }
    return true;
}

std::shared_ptr<const ZXing::BitMatrix> QImagePureBinarizer::getBlackMatrix() const
{
    using namespace ZXing;

    if (!m_bitmap) {
        const auto w = width();
        const auto h = height();

        auto bitmap = std::make_shared<BitMatrix>(w, h);
        for (int x = 1; x < w - 1; ++x) {
            for (int y = 1; y < h - 1; ++y) {
                if (qGray(m_img.pixel(x - 1, y - 1)) < 127)
                    bitmap->set(x, y);
            }
        }

        m_bitmap = std::move(bitmap);
    }

    return m_bitmap;
}

bool QImagePureBinarizer::canCrop() const
{
    return false; // unused by ZXing
}

std::shared_ptr<ZXing::BinaryBitmap> QImagePureBinarizer::cropped(int left, int top, int width, int height) const
{
    Q_UNUSED(left);
    Q_UNUSED(top);
    Q_UNUSED(width);
    Q_UNUSED(height);
    return {};
}

bool QImagePureBinarizer::canRotate() const
{
    return false;
}

std::shared_ptr<ZXing::BinaryBitmap> QImagePureBinarizer::rotated(int degreeCW) const
{
    Q_UNUSED(degreeCW);
    return {};
}

#endif
