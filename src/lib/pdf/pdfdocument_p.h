/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <config-kitinerary.h>

#include <QImage>

#include <memory>
#include <unordered_map>
#include <vector>

class QString;

class PDFDoc;

namespace KItinerary {

class PdfDocumentPrivate;
class PdfImage;
class PdfPage;

class PdfPagePrivate : public QSharedData {
public:
    void load();

    int m_pageNum = -1;
    bool m_loaded = false;
    QString m_text;
    std::vector<PdfImage> m_images;
    PdfDocumentPrivate *m_doc;
};

class PdfDocumentPrivate {
public:
    // needs to be kept alive as long as the Poppler::PdfDoc instance lives
    QByteArray m_pdfData;
    // this contains the actually loaded/decoded image data
    // and is referenced by the object id from PdfImage to avoid
    // expensive loading/decoding of multiple occurrences of the same image
    // image data in here is stored in its source form, without applied transformations
    std::unordered_map<int, QImage> m_imageData;
    std::vector<PdfPage> m_pages;
    std::unique_ptr<PDFDoc> m_popplerDoc;
};

}


