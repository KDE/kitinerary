/*
    SPDX-FileCopyrightText: 2018-2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "pdfbarcodeutil_p.h"
#include "pdfimage.h"

using namespace KItinerary;

enum {
    // unit is 1/72 inch, assuming landscape orientation
    MinTargetImageHeight = 28,
    MinTargetImageWidth = 36,
    MaxTargetImageHeight = 252,
    MaxTargetImageWidth = 252,
};

bool PdfBarcodeUtil::maybeBarcode(const PdfImage &img, BarcodeDecoder::BarcodeTypes hint)
{
    const auto w = img.width();
    const auto h = img.height();

    if (!BarcodeDecoder::isPlausibleSize(img.sourceWidth(), img.sourceHeight(), hint) || !BarcodeDecoder::isPlausibleAspectRatio(w, h, hint)) {
        return false;
    }

    // image target size checks
    if (std::min(w, h) < MinTargetImageHeight || std::max(w, h) < MinTargetImageWidth || h > MaxTargetImageHeight || w > MaxTargetImageWidth) {
        return false;
    }

    return true;
}
