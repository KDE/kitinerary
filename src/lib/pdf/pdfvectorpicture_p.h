/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QExplicitlySharedDataPointer>

#include <QPainterPath>
#include <QPen>
#include <QBrush>

class QTransform;

namespace KItinerary {

class PdfVectorPicturePrivate;

/** Similar to QPicture, for deferred-rendered path-based vector graphics extracted from PDF documents. */
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

    // transform applied to this for displaying
    QTransform transform() const;
    void setTransform(const QTransform &t);

    QRectF boundingRect() const;
    int pathElementsCount() const;
    QImage renderToImage() const;

    // size of the rendered image
    int sourceWidth() const;
    int sourceHeight() const;
    // size of the image in PDF 1/72 dpi coordinates
    int width() const;
    int height() const;

private:
    QExplicitlySharedDataPointer<PdfVectorPicturePrivate> d;
};

}

