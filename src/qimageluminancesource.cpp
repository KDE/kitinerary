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

#include "qimageluminancesource.h"
#ifdef HAVE_ZXING_OLD

using namespace KItinerary;

QImageLuminanceSource::QImageLuminanceSource(const QImage &img)
    : zxing::LuminanceSource(img.width() + 2, img.height() + 2)
    , m_img(img)
{
}

QImageLuminanceSource::~QImageLuminanceSource() = default;

zxing::ArrayRef<char> QImageLuminanceSource::getRow(int y, zxing::ArrayRef<char> row) const
{
    if (!row) {
        row = zxing::ArrayRef<char>(getWidth());
    }

    if (y == 0 || y == getHeight() - 1) {
        memset(&row[0], 0xff, getWidth());
        return row;
    }

    row[0] = (char)0xff;
    for (int i = 1; i < getWidth() - 1; ++i) {
        row[i] = luminance(i - 1, y);
    }
    row[getWidth() - 1] = (char)0xff;

    return row;
}

zxing::ArrayRef<char> QImageLuminanceSource::getMatrix() const
{
    zxing::ArrayRef<char> matrix(getWidth() * getHeight());

    memset(&matrix[0], 0xff, getWidth());
    for (int i = 1; i < getHeight() - 1; ++i) {
        matrix[i * getWidth()] = (char)0xff;
        for (int j = 1; j < getWidth() - 1; ++j) {
            matrix[i * getWidth() + j] = luminance(j - 1, i - 1);
        }
        matrix[(i + 1) * getWidth() - 1] = (char)0xff;
    }
    memset(&matrix[(getHeight() - 1) * getWidth()], 0xff, getWidth());

    return matrix;
}

char QImageLuminanceSource::luminance(int x, int y) const
{
    return qGray(m_img.pixel(x, y));
}

#endif
