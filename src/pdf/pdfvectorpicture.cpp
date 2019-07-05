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

#include "pdfvectorpicture_p.h"

#include <QDebug>
#include <QImage>
#include <QPainter>

#include <cmath>

enum { RenderDPI = 150 }; // target dpi for rendering

using namespace KItinerary;

namespace KItinerary {
class PdfVectorPicturePrivate : public QSharedData {
public:
    std::vector<PdfVectorPicture::PathStroke> strokes;
    QRectF boundingRect;
    QImage image;
};
}

PdfVectorPicture::PdfVectorPicture()
    : d(new PdfVectorPicturePrivate)
{
}

PdfVectorPicture::PdfVectorPicture(const PdfVectorPicture&) = default;
PdfVectorPicture::~PdfVectorPicture() = default;
PdfVectorPicture& PdfVectorPicture::operator=(const PdfVectorPicture&) = default;

void PdfVectorPicture::setStrokes(std::vector<PdfVectorPicture::PathStroke> &&strokes)
{
    d.detach();
    d->strokes = std::move(strokes);
    d->image = QImage();
    d->boundingRect = QRectF();
}

QRectF PdfVectorPicture::boundingRect() const
{
    if (d->strokes.empty()) {
        return {};
    }
    if (d->boundingRect.isEmpty()) {
        for (const auto &stroke : d->strokes) {
            d->boundingRect = d->boundingRect.united(stroke.path.boundingRect());
        }
    }

    return d->boundingRect;
}

int PdfVectorPicture::pathElementsCount() const
{
    int c = 0;
    for (const auto &stroke : d->strokes) {
        c += stroke.path.elementCount();
    }
    return c;
}

QImage PdfVectorPicture::renderToImage(int dpi) const
{
    if (d->image.isNull()) {
        const auto scale = dpi / 72; // 1/72 dpi is the unit for the vector coordinates
        const int width = boundingRect().width() * scale;
        const int height = boundingRect().height() * scale;
        d->image = QImage(width, height, QImage::Format_Grayscale8);
        d->image.fill(Qt::white);
        QPainter p(&d->image);
        p.scale(scale, scale);
        for (const auto &stroke : d->strokes) {
            if (stroke.brush.style() == Qt::NoBrush) {
                p.strokePath(stroke.path.translated(-boundingRect().x(), -boundingRect().y()), stroke.pen);
            } else {
                p.fillPath(stroke.path.translated(-boundingRect().x(), -boundingRect().y()), stroke.brush);
            }
        }
    }

    return d->image;
}
