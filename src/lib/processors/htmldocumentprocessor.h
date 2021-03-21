/*
   SPDX-FileCopyrightText: 2017-2021 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_HTMLDOCUMENTPROCESSOR_H
#define KITINERARY_HTMLDOCUMENTPROCESSOR_H

#include <KItinerary/ExtractorDocumentProcessor>

namespace KItinerary {

/** Processor for HTML documents. */
class HtmlDocumentProcessor : public ExtractorDocumentProcessor
{
public:
    bool canHandleData(const QByteArray &encodedData, QStringView fileName) const override;
    ExtractorDocumentNode createNodeFromData(const QByteArray &encodedData) const override;
    void preExtract(ExtractorDocumentNode &node, const ExtractorEngine *engine) const override;
    QJSValue contentToScriptValue(const ExtractorDocumentNode &node, QJSEngine *engine) const override;
    void destroyNode(ExtractorDocumentNode &node) const override;
};

}

#endif // KITINERARY_HTMLDOCUMENTPROCESSOR_H
