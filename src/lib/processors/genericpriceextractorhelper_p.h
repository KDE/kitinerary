/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_GENERICPRICEEXTRACTORHELPER_P_H
#define KITINERARY_GENERICPRICEEXTRACTORHELPER_P_H

#include <barcodedecoder.h>

namespace KItinerary {

class ExtractorDocumentNode;

/** Generic price extractor code used by multiple document processors. */
namespace GenericPriceExtractorHelper
{
void postExtract(const QString &text, ExtractorDocumentNode &node);
}

}

#endif // KITINERARY_GENERICPRICEEXTRACTORHELPER_P_H
