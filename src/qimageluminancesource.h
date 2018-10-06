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

#ifndef KITINERARY_QIMAGELUMINANCESOURCE_H
#define KITINERARY_QIMAGELUMINANCESOURCE_H

#include "config-kitinerary.h"
#ifdef HAVE_ZXING

#include <QImage>

#include <zxing/LuminanceSource.h>

namespace KItinerary {

/** QImage-based LuminanceSource.
 *  This automatically adds a 1px quiet zone around the content.
 */
class QImageLuminanceSource : public zxing::LuminanceSource
{
public:
    explicit QImageLuminanceSource(const QImage &img);
    ~QImageLuminanceSource();

    zxing::ArrayRef<char> getRow(int y, zxing::ArrayRef<char> row) const override;
    zxing::ArrayRef<char> getMatrix() const override;

private:
    char luminance(int x, int y) const;

    QImage m_img;
};

}

#endif

#endif // KITINERARY_QIMAGELUMINANCESOURCE_H
