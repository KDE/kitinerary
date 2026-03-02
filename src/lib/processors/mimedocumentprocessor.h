/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KItinerary/ExtractorDocumentProcessor>

namespace KItinerary {

/** MIME document processor. */
class MimeDocumentProcessor : public ExtractorDocumentProcessor
{
public:
    [[nodiscard]] bool canHandleData(const QByteArray &encodedData, QStringView fileName) const override;
    [[nodiscard]] ExtractorDocumentNode createNodeFromData(const QByteArray &encodedData) const override;
    [[nodiscard]] ExtractorDocumentNode createNodeFromContent(const QVariant &decodedData) const override;
    void expandNode(ExtractorDocumentNode& node, const ExtractorEngine *engine) const override;
    [[nodiscard]] bool matches(const ExtractorFilter &filter, const ExtractorDocumentNode &node) const override;
    void destroyNode(ExtractorDocumentNode &node) const override;
    [[nodiscard]] QJSValue contentToScriptValue(const ExtractorDocumentNode &node, QJSEngine *engine) const override;
};

}

