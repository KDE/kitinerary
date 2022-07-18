/*
    SPDX-FileCopyrightText: 2018-2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "barcodedecoder.h"

namespace KItinerary {

class PdfImage;

/** Helper functions for detecting barcodes in PDF data. */
namespace PdfBarcodeUtil
{
    /** Quick pre-check without image decoding if @p img might be a barcode. */
    BarcodeDecoder::BarcodeTypes maybeBarcode(const PdfImage &img, BarcodeDecoder::BarcodeTypes hint);

    /** Check whether a vector path with @p elementCount could be a barcode. */
    BarcodeDecoder::BarcodeTypes isPlausiblePath(int elementCount, BarcodeDecoder::BarcodeTypes hint);
}

}

