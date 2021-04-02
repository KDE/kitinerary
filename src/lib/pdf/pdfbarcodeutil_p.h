/*
    SPDX-FileCopyrightText: 2018-2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_PDFBARCODEUTIL_H
#define KITINERARY_PDFBARCODEUTIL_H

#include "barcodedecoder.h"

namespace KItinerary {

class PdfImage;

/** Helper functions for detecting barcodes in PDF data. */
namespace PdfBarcodeUtil
{
    /** Quick pre-check without image decoding if @p img might be a barcode. */
    bool maybeBarcode(const PdfImage &img, BarcodeDecoder::BarcodeTypes hint = BarcodeDecoder::Any);
}

}

#endif // KITINERARY_PDFBARCODEUTIL_H
