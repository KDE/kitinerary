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

#ifndef KITINERARY_PDFDOCUMENT_P_H
#define KITINERARY_PDFDOCUMENT_P_H

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
#ifdef HAVE_POPPLER
    std::unique_ptr<PDFDoc> m_popplerDoc;
#endif
};

}

#endif

