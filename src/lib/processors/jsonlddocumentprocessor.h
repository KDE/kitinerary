/*
   SPDX-FileCopyrightText: 2017-2021 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_JSONLDDOCUMENTPROCESSOR_H
#define KITINERARY_JSONLDDOCUMENTPROCESSOR_H

#include <KItinerary/ExtractorDocumentProcessor>

namespace KItinerary {

/** Processor for raw JSON-LD data.
 *  This is simply a pass-through for JSON data, so the using code can apply post-processing to that.
 */
class JsonLdDocumentProcessor : public ExtractorDocumentProcessor
{
public:
    bool canHandleData(const QByteArray &encodedData, QStringView fileName) const override;
    ExtractorDocumentNode createNodeFromData(const QByteArray &encodedData) const override;
    void preExtract(ExtractorDocumentNode &node, const ExtractorEngine *engine) const override;
};

}

#endif // KITINERARY_JSONLDDOCUMENTPROCESSOR_H
