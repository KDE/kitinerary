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
    bool canHandleData(const QByteArray &encodedData, QStringView fileName) const override;
    ExtractorDocumentNode createNodeFromData(const QByteArray &encodedData) const override;
    ExtractorDocumentNode createNodeFromContent(const QVariant &decodedData) const override;
    void expandNode(ExtractorDocumentNode& node, const ExtractorEngine *engine) const override;
    bool matches(const ExtractorFilter &filter, const ExtractorDocumentNode &node) const override;
    void destroyNode(ExtractorDocumentNode &node) const override;
};

}

