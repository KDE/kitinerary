/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_PDFLINK_H
#define KITINERARY_PDFLINK_H

#include "kitinerary_export.h"

#include <QExplicitlySharedDataPointer>
#include <QMetaType>
#include <QRectF>

class PDFRectangle;

namespace KItinerary {

class PdfExtractorOutputDevice;
class PdfLinkPrivate;
class PdfPagePrivate;

/** An external link in a PDF file. */
class KITINERARY_EXPORT PdfLink
{
    Q_GADGET
    Q_PROPERTY(QString url READ url)
    Q_PROPERTY(QRectF area READ area)

public:
    PdfLink();
    ~PdfLink();
    PdfLink(const PdfLink&);
    PdfLink& operator=(const PdfLink&);

    QString url() const;
    QRectF area() const;

private:
    friend class PdfExtractorOutputDevice;
    friend class PdfPagePrivate;

    explicit PdfLink(const QString &url, const QRectF &area);
    void convertToPageRect(const PDFRectangle *pageRect);

    QExplicitlySharedDataPointer<PdfLinkPrivate> d;
};

}

#endif // KITINERARY_PDFLINK_H
