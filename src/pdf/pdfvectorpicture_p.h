/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

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

#ifndef KITINERARY_PDFVECTORPICTURE_P_H
#define KITINERARY_PDFVECTORPICTURE_P_H

#include <QExplicitlySharedDataPointer>

#include <QPainterPath>
#include <QPen>
#include <QBrush>

namespace KItinerary {

class PdfVectorPicturePrivate;

/** Similar to QPicture, for defered-rendered path-based vector graphics extracted from PDF documents. */
class PdfVectorPicture
{
public:
    PdfVectorPicture();
    PdfVectorPicture(const PdfVectorPicture&);
    ~PdfVectorPicture();
    PdfVectorPicture& operator=(const PdfVectorPicture&);

    struct PathStroke {
        QPainterPath path;
        QPen pen;
        QBrush brush;
    };

    void setStrokes(std::vector<PathStroke> &&strokes);
    QRectF boundingRect() const;
    int pathElementsCount() const;
    QImage renderToImage(int dpi) const;

private:
    QExplicitlySharedDataPointer<PdfVectorPicturePrivate> d;
};

}

#endif // KITINERARY_PDFVECTORPICTURE_H
