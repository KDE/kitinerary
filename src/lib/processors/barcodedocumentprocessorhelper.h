/*
    SPDX-FileCopyrightText: 2021-2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_BARCODEDOCUMENTPROCESSORHELPER_H
#define KITINERARY_BARCODEDOCUMENTPROCESSORHELPER_H

#include <barcodedecoder.h>

namespace KItinerary {

class ExtractorDocumentNode;
class ExtractorEngine;

/** Barcode-related document processor implementation details. */
namespace BarcodeDocumentProcessorHelper
{
void expandNode(const QImage &img, BarcodeDecoder::BarcodeTypes barcodeHints, ExtractorDocumentNode &parent, const ExtractorEngine *engine);
}

}

#endif // KITINERARY_BARCODEDOCUMENTPROCESSORHELPER_H
