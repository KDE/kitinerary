/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_HTTPRESPONSEPROCESSOR_H
#define KITINERARY_HTTPRESPONSEPROCESSOR_H

#include <KItinerary/ExtractorDocumentProcessor>

namespace KItinerary {

/** Processor for HTTP responses. */
class HttpResponseProcessor : public ExtractorDocumentProcessor
{
public:
    void expandNode(ExtractorDocumentNode &node, const ExtractorEngine *engine) const override;
};

/** Processor for entire HAR archives.
 *  This is only relevant for development tooling.
 */
class HarDocumentProcessor : public ExtractorDocumentProcessor
{
public:
    bool canHandleData(const QByteArray & encodedData, QStringView fileName) const override;
    ExtractorDocumentNode createNodeFromData(const QByteArray &encodedData) const override;
    void expandNode(ExtractorDocumentNode & node, const ExtractorEngine * engine) const override;
};

}

#endif // KITINERARY_HTTPRESPONSEPROCESSOR_H
