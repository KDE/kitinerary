/*
   SPDX-FileCopyrightText: 2017-2021 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KItinerary/ExtractorDocumentProcessor>

namespace KItinerary {

class HtmlElement;

/** Processor for HTML documents. */
class HtmlDocumentProcessor : public ExtractorDocumentProcessor
{
public:
    bool canHandleData(const QByteArray &encodedData, QStringView fileName) const override;
    ExtractorDocumentNode createNodeFromData(const QByteArray &encodedData) const override;
    ExtractorDocumentNode createNodeFromContent(const QVariant& decodedData) const override;
    void expandNode(ExtractorDocumentNode &node, const ExtractorEngine *engine) const override;
    void preExtract(ExtractorDocumentNode &node, const ExtractorEngine *engine) const override;
    void postExtract(ExtractorDocumentNode &node, const ExtractorEngine *engine) const override;
    QJSValue contentToScriptValue(const ExtractorDocumentNode &node, QJSEngine *engine) const override;
    void destroyNode(ExtractorDocumentNode &node) const override;

private:
    void expandElementRecursive(ExtractorDocumentNode &node, const HtmlElement &elem, const ExtractorEngine *engine) const;
    void expandDataUrl(ExtractorDocumentNode &node, QStringView data, const ExtractorEngine *engine) const;
};

}

