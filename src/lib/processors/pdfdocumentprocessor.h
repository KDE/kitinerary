/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KItinerary/ExtractorDocumentProcessor>
#include <KItinerary/PdfImage>

#include <unordered_set>

namespace KItinerary {

/** PDF document processor. */
class PdfDocumentProcessor : public ExtractorDocumentProcessor
{
public:
    PdfDocumentProcessor();
    ~PdfDocumentProcessor();

    bool canHandleData(const QByteArray &encodedData, QStringView fileName) const override;
    ExtractorDocumentNode createNodeFromData(const QByteArray &encodedData) const override;
    ExtractorDocumentNode createNodeFromContent(const QVariant &decodedData) const override;
    void expandNode(ExtractorDocumentNode &node, const ExtractorEngine *engine) const override;
    void postExtract(ExtractorDocumentNode &node) const;
    QJSValue contentToScriptValue(const ExtractorDocumentNode &node, QJSEngine *engine) const override;
    void destroyNode(ExtractorDocumentNode &node) const override;

private:
    mutable std::unordered_set<PdfImageRef> m_imageIds;
};

}

