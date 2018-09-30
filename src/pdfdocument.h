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

#ifndef KITINERARY_PDFDOCUMENT_H
#define KITINERARY_PDFDOCUMENT_H

#include "kitinerary_export.h"

#include <QExplicitlySharedDataPointer>
#include <QObject>
#include <QVariant>

#include <memory>

class QImage;
class QTransform;

namespace KItinerary {

class PdfImagePrivate;
class PdfPagePrivate;

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

    int width() const;
    int height() const;

    /** Height of the source image. */
    int sourceHeight() const;
    /** Width of the source image. */
    int sourceWidth() const;

    /** Transformation from source image to final size/position on the page.
     *  Values are 1/72 inch.
     */
    QTransform transform() const;

    /** The image as loaded from PDF, without transformations applied. */
    QImage sourceImage() const;
    /** The source image with display transformations applied. */
    QImage image() const;

    /** PDF-internal unique identifier of this image.
     *  Use this to detect multiple occurrences of the same image in different
     *  places, if that reduces e.g. computation cost.
     */
    int objectId() const;

private:
    friend class ExtractorOutputDevice;
    friend class PdfPagePrivate;
    friend class PdfPage;
    QExplicitlySharedDataPointer<PdfImagePrivate> d;
};


/** A page in a PDF document.
 */
class KITINERARY_EXPORT PdfPage
{
    Q_GADGET
    Q_PROPERTY(QString text READ text)
    Q_PROPERTY(QVariantList images READ imagesVariant)
public:
    PdfPage();
    PdfPage(const PdfPage&);
    ~PdfPage();
    PdfPage& operator=(const PdfPage&);

    /** The entire text on this page. */
    QString text() const;

    /** Returns the text in the specified sub-rect of this page.
     *  All parameters are relative values between @c 0 and @c 1 of the entire page size.
     */
    Q_INVOKABLE QString textInRect(double left, double top, double right, double bottom) const;

    /** The number of images found in this document. */
    int imageCount() const;

    /** The n-th image found in this document. */
    PdfImage image(int index) const;

    /** Returns the images in the specified sub-rect of this page.
     *  All parameters are relative values between @c 0 and @c 1 of the entire page size.
     */
    Q_INVOKABLE QVariantList imagesInRect(double left, double top, double right, double bottom) const;

private:
    QVariantList imagesVariant() const;

    friend class PdfDocument;
    QExplicitlySharedDataPointer<PdfPagePrivate> d;
};


class PdfDocumentPrivate;

/** PDF document for extraction.
 *  This is used as input for ExtractorEngine and the JS extractor scripts.
 *  @note This class is only functional if Poppler is available as a dependency,
 *  otherwise all methods return empty values.
 */
class KITINERARY_EXPORT PdfDocument : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text CONSTANT)
    Q_PROPERTY(int pageCount READ pageCount CONSTANT)
    Q_PROPERTY(QVariantList pages READ pagesVariant CONSTANT)
public:
    explicit PdfDocument(QObject *parent = nullptr);
    ~PdfDocument();

    /** The entire text extracted from the PDF document. */
    QString text() const;

    /** The number of pages in this document. */
    int pageCount() const;

    /** The n-thj page in this document. */
    PdfPage page(int index) const;

    /** File size of the entire document in bytes. */
    int fileSize() const;

    /** Creates a PdfDocument from the given raw data.
     *  @returns @c nullptr if loading fails or Poppler was not found.
     */
    static PdfDocument* fromData(const QByteArray &data, QObject *parent = nullptr);

private:
    QVariantList pagesVariant() const;

    std::unique_ptr<PdfDocumentPrivate> d;
};

}

Q_DECLARE_METATYPE(KItinerary::PdfImage)
Q_DECLARE_METATYPE(KItinerary::PdfPage)

#endif // KITINERARY_PDFDOCUMENT_H
