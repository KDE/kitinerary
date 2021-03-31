/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "genericextractor_p.h"

#include <KItinerary/BarcodeDecoder>

#include <QDateTime>

#include <unordered_set>
#include <vector>


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

    /** Try to extract the given document. */
    std::vector<GenericExtractor::Result> extract(PdfDocument *doc);

    /** Quick pre-check without image decoding if @p img might be a barcode. */
    static bool maybeBarcode(const PdfImage &img, BarcodeDecoder::BarcodeTypes hint = BarcodeDecoder::Any);

private:
    GenericExtractor::Result extractImage(const PdfImage &img, const std::vector<GenericExtractor::Result> &existingResults);

    QDateTime m_contextDate;
    std::unordered_set<int> m_imageIds;
    BarcodeDecoder *m_barcodeDecoder = nullptr;
};

}

