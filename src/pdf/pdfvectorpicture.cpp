/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
    QTransform transform;
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

QTransform PdfVectorPicture::transform() const
{
    return d->transform;
}

void PdfVectorPicture::setTransform(const QTransform &t)
{
    d.detach();
    d->transform = t;
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

static double scaleFromTransform(QTransform t)
{
    if (t.isRotating()) {
        t.rotate(90);
        if (t.isRotating()) {
            qDebug() << "non-90° rotation, likely not a barcode";
            return 1.0;
        }
    }

    if (std::abs(std::abs(t.m11()) - std::abs(t.m22())) < 0.1) {
        return std::max(std::abs(t.m11()), std::abs(t.m22()));
    }
    qDebug() << "asymetric scale not supported yet" << t;
    return 1.0;
}

QImage PdfVectorPicture::renderToImage() const
{
    if (d->image.isNull()) {
        const double scale = (RenderDPI / 72.0) * scaleFromTransform(d->transform); // 1/72 dpi is the unit for the vector coordinates
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

int PdfVectorPicture::sourceWidth() const
{
    // 1/72 dpi is the unit for the vector coordinates
    return boundingRect().width() * (RenderDPI / 72.0);
}

int PdfVectorPicture::sourceHeight() const
{
    // 1/72 dpi is the unit for the vector coordinates
    return boundingRect().width() * (RenderDPI / 72.0);
}

int PdfVectorPicture::width() const
{
    return boundingRect().width() * scaleFromTransform(d->transform);
}

int PdfVectorPicture::height() const
{
    return boundingRect().height() * scaleFromTransform(d->transform);
}
