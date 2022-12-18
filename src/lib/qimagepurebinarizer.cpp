/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "qimagepurebinarizer.h"
#if !ZXING_USE_READBARCODE

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
    // until ZXing 1.0.6 returning true here works, after d57cbe2121bcc761474b0f605bdfe0fa1fec676a in zxing
    // this however breaks QR decoding
    // the performance gain seems minimal though (not used by PDF417 and Aztec decoding),
    // so we can just set this to false unconditioanlly
    return false;
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

#if ZXING_VERSION >= QT_VERSION_CHECK(1, 1, 1)
bool QImagePureBinarizer::getPatternRow(int y, ZXing::PatternRow& res) const
{
    res.clear();
    // TODO we can probably skip this indirection eventually and directly access image data here?
    ZXing::BitArray row;
    getBlackRow(y, row);

    auto li = row.begin();
    auto i = li;
    if (*i) {
        res.push_back(0);
    }
    while ((i = row.getNextSetTo(i, !*i)) != row.end()) {
        res.push_back(static_cast<ZXing::PatternRow::value_type>(i - li));
        li = i;
    }
    res.push_back(static_cast<ZXing::PatternRow::value_type>(i - li));
    return true;
}
#endif

std::shared_ptr<const ZXing::BitMatrix> QImagePureBinarizer::getBlackMatrix() const
{
    using namespace ZXing;

    if (!m_bitmap) {
        const auto w = width();
        const auto h = height();
        const auto bpp = m_img.bytesPerLine() / (w - 2);

        auto bitmap = std::make_shared<BitMatrix>(w, h);
        // fast path for grayscale, avoids the conversion to qRGB inside pixel() and uses the faster scanline access
        if (bpp == 1) {
            for (int y = 1; y < h - 1; ++y) {
                const auto scanline = m_img.scanLine(y - 1);
                for (int x = 1; x < w - 1; ++x) {
                    if (scanline[(x - 1)] < 127)
                        bitmap->set(x, y);
                }
            }
        } else {
            for (int x = 1; x < w - 1; ++x) {
                for (int y = 1; y < h - 1; ++y) {
                    if (qGray(m_img.pixel(x - 1, y - 1)) < 127)
                        bitmap->set(x, y);
                }
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
    Q_UNUSED(left)
    Q_UNUSED(top)
    Q_UNUSED(width)
    Q_UNUSED(height)
    return {};
}

bool QImagePureBinarizer::canRotate() const
{
    return false;
}

std::shared_ptr<ZXing::BinaryBitmap> QImagePureBinarizer::rotated(int degreeCW) const
{
    Q_UNUSED(degreeCW)
    return {};
}

#endif
