/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kitinerary_export.h"

#include <KItinerary/PdfImage>

#include <QDateTime>
#include <QExplicitlySharedDataPointer>
#include <QObject>
#include <QVariant>

#include <memory>


namespace KItinerary {

class PdfLink;
class PdfPagePrivate;

/** A page in a PDF document.
 */
class KITINERARY_EXPORT PdfPage
{
    Q_GADGET
    Q_PROPERTY(QString text READ text)
    Q_PROPERTY(QVariantList images READ imagesVariant)
    Q_PROPERTY(QVariantList links READ linksVariant)
    Q_PROPERTY(int width READ width)
    Q_PROPERTY(int height READ height)
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

    /** The number of links found in this document. */
    int linkCount() const;

    /** The n-th link found in this document. */
    PdfLink link(int index) const;

    /** Returns all links in the specified sub-rect of this page.
     *  All parameters are relative values between @c 0 and @c 1 of the entire page size.
     *  Links are sorted top to bottom / left to right based on their top left corner.
     */
    Q_INVOKABLE QVariantList linksInRect(double left, double top, double right, double bottom) const;

    /** Page size in millimeters. */
    int width() const;
    int height() const;

private:
    QVariantList imagesVariant() const;
    QVariantList linksVariant() const;

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
    Q_PROPERTY(QString title READ title CONSTANT)
    Q_PROPERTY(QString producer READ producer CONSTANT)

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

    /** The document title. */
    QString title() const;
    /** The document producer. */
    QString producer() const;

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

