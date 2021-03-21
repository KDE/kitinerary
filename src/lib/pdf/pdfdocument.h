/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_PDFDOCUMENT_H
#define KITINERARY_PDFDOCUMENT_H

#include "kitinerary_export.h"

#include <KItinerary/PdfImage>

#include <QDateTime>
#include <QExplicitlySharedDataPointer>
#include <QObject>
#include <QVariant>

#include <memory>


namespace KItinerary {

class PdfPagePrivate;

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
    Q_PROPERTY(QDateTime creationTime READ creationTime CONSTANT)
    Q_PROPERTY(QDateTime modificationTime READ modificationTime CONSTANT)

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

    /** Creation time as specified in the PDF file. */
    QDateTime creationTime() const;
    /** Modification time as specified in the PDF file. */
    QDateTime modificationTime() const;

    /** Creates a PdfDocument from the given raw data.
     *  @returns @c nullptr if loading fails or Poppler was not found.
     */
    static PdfDocument* fromData(const QByteArray &data, QObject *parent = nullptr);

    /** Fast check whether @p data might be a PDF document. */
    static bool maybePdf(const QByteArray &data);

private:
    QVariantList pagesVariant() const;

    std::unique_ptr<PdfDocumentPrivate> d;
};

}

Q_DECLARE_METATYPE(KItinerary::PdfPage)

#endif // KITINERARY_PDFDOCUMENT_H
