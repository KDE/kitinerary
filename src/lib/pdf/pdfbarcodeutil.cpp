/*
    SPDX-FileCopyrightText: 2018-2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "pdfbarcodeutil_p.h"
#include "pdfimage.h"

using namespace KItinerary;

enum {
    // unit is 1/72 inch, assuming landscape orientation
    MinTargetImageHeight2D = 28,
    MinTargetImageHeight1D = 20,
    MinTargetImageWidth = 36,
    MaxTargetImageHeight = 252,
    MaxTargetImageWidth2D = 252,
    MaxTargetImageWidth1D = 272,
    // vector path complexity limits
    MinPathElementCount2D = 200,
    MaxPathElementCount2D = 20000, // ÖBB got creative with vector barcodes...
    MinPathElementCount1D = 150,
    MaxPathElementCount1D = 400,
};

BarcodeDecoder::BarcodeTypes PdfBarcodeUtil::maybeBarcode(const PdfImage &img, BarcodeDecoder::BarcodeTypes hint)
{
    const auto w = img.width();
    const auto h = img.height();

    // image target size checks
    if (std::max(w, h) < MinTargetImageWidth || std::min(w, h) > MaxTargetImageHeight) {
        return BarcodeDecoder::None;
    }

    if (std::max(w, h) > MaxTargetImageWidth2D || std::min(w, h) < MinTargetImageHeight2D) {
        hint &= ~BarcodeDecoder::Any2D;
    }
    if (std::max(w, h) > MaxTargetImageWidth1D || std::min(w, h) < MinTargetImageHeight1D) {
        hint &= ~BarcodeDecoder::Any1D;
    }

    hint = BarcodeDecoder::isPlausibleSize(img.sourceWidth(), img.sourceHeight(), hint);
    hint = BarcodeDecoder::isPlausibleAspectRatio(w, h, hint);

    if (img.isVectorImage()) {
        hint = isPlausiblePath(img.pathElementsCount(), hint);
    }

    return hint;
}

BarcodeDecoder::BarcodeTypes PdfBarcodeUtil::isPlausiblePath(int elementCount, BarcodeDecoder::BarcodeTypes hint)
{
    if (elementCount < MinPathElementCount2D || elementCount > MaxPathElementCount2D) {
        hint &= ~BarcodeDecoder::Any2D;
    }
    if (elementCount < MinPathElementCount1D || elementCount > MaxPathElementCount1D) {
        hint &= ~BarcodeDecoder::Any1D;
    }
    return hint;
}
