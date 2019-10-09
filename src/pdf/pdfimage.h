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

#ifndef KITINERARY_PDFIMAGE_H
#define KITINERARY_PDFIMAGE_H

#include "kitinerary_export.h"

#include <QExplicitlySharedDataPointer>
#include <QMetaType>

class QImage;
class QTransform;

namespace KItinerary {

class PdfImagePrivate;

/** An image in a PDF document.
 */
class KITINERARY_EXPORT PdfImage
{
    Q_GADGET
    Q_PROPERTY(int width READ width)
    Q_PROPERTY(int height READ height)
public:
    PdfImage();
    PdfImage(const PdfImage&);
    ~PdfImage();
    PdfImage& operator=(const PdfImage&);

    /** Width of the image in PDF 1/72 dpi coordinates. */
    int width() const;
    /** Height of the image in PDF 1/72 dpi coordinates. */
    int height() const;

    /** Height of the source image. */
    int sourceHeight() const;
    /** Width of the source image. */
    int sourceWidth() const;

    /** Transformation from source image to final size/position on the page.
     *  Values are 1/72 inch.
     */
    QTransform transform() const;

    /** Hints for loading image data. */
    enum LoadingHint {
        NoHint = 0, ///< Load image data as-is. The default.
        AbortOnColorHint = 1, ///< Abort loading when encountering a non black/white pixel, as a shortcut for barcode detection.
    };
    Q_DECLARE_FLAGS(LoadingHints, LoadingHint)

    /** Sets image loading hints. */
    void setLoadingHints(LoadingHints hints);

    /** The source image with display transformations applied. */
    QImage image() const;

    /** Returns whether this image has an object id.
     *  Vector graphic "images" don't have that.
     */
    bool hasObjectId() const;

    /** PDF-internal unique identifier of this image.
     *  Use this to detect multiple occurrences of the same image in different
     *  places, if that reduces e.g. computation cost.
     */
    int objectId() const;

private:
    friend class PdfExtractorOutputDevice;
    friend class PdfPagePrivate;
    friend class PdfPage;
    QExplicitlySharedDataPointer<PdfImagePrivate> d;
};

}

Q_DECLARE_METATYPE(KItinerary::PdfImage)
Q_DECLARE_OPERATORS_FOR_FLAGS(KItinerary::PdfImage::LoadingHints)

#endif // KITINERARY_PDFIMAGE_H
