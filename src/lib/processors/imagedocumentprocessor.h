/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_IMAGEDOCUMENTPROCESSOR_H
#define KITINERARY_IMAGEDOCUMENTPROCESSOR_H

#include <KItinerary/ExtractorDocumentProcessor>

namespace KItinerary {

/** Processor for image document elements. */
class ImageDocumentProcessor : public ExtractorDocumentProcessor
{
public:
    void expandNode(ExtractorDocumentNode &node, const ExtractorEngine *engine) const override;
};

}

#endif // KITINERARY_IMAGEDOCUMENTPROCESSOR_H
