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
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef KITINERARY_GENERICPDFEXTRACTOR_P_H
#define KITINERARY_GENERICPDFEXTRACTOR_P_H

#include <KItinerary/BarcodeDecoder>

#include <QDateTime>
#include <QJsonArray>

#include <unordered_set>
#include <vector>

class QJsonArray;
class QString;

namespace KItinerary {

class BarcodeDecoder;
class PdfDocument;
class PdfImage;

/** Generic extractor for PDF documents.
 *  This is applied to all PDF documents and searches for
 *  barcodes we can recognize.
 *
 *  @internal
 */
class GenericPdfExtractor
{
public:
    GenericPdfExtractor();
    ~GenericPdfExtractor();
    GenericPdfExtractor(const GenericPdfExtractor&) = delete;
    void setBarcodeDecoder(BarcodeDecoder *decoder);

    /** Set the context date used for extraction. */
    void setContextDate(const QDateTime &dt);

    /** PDF extraction result. */
    struct Result {
        int pageNum = -1; // page number, if result is from a single page
        QJsonArray result; // JSON-LD data extracted from this document or page
        QVariant barcode; // unrecognized barcode for further processing
    };

    /** Try to extract the given document. */
    std::vector<Result> extract(PdfDocument *doc);

    /** Quick pre-check without image decoding if @p img might be a barcode. */
    static bool maybeBarcode(const PdfImage &img, BarcodeDecoder::BarcodeTypes hint = BarcodeDecoder::Any);

private:
    Result extractImage(const PdfImage &img);
    Result extractBarcode(const QString &code);

    QDateTime m_contextDate;
    std::unordered_set<int> m_imageIds;
    BarcodeDecoder *m_barcodeDecoder = nullptr;
};

}

#endif // KITINERARY_GENERICPDFEXTRACTOR_P_H
