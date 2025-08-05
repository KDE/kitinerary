/*
   SPDX-FileCopyrightText: ⓒ 2025 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_PKPASSESDOCUMENTPROCESSOR_H
#define KITINERARY_PKPASSESDOCUMENTPROCESSOR_H

#include <KItinerary/ExtractorDocumentProcessor>

namespace KItinerary {

/** Processor for Apple Wallet pass files. */
class PkPassesDocumentProcessor : public ExtractorDocumentProcessor
{
public:
    [[nodiscard]] bool canHandleData(const QByteArray &encodedData, QStringView fileName) const override;
    [[nodiscard]] ExtractorDocumentNode createNodeFromData(const QByteArray &encodedData) const override;
    void expandNode(ExtractorDocumentNode &node, const ExtractorEngine *engine) const override;
    void destroyNode(ExtractorDocumentNode &node) const override;
};

}

#endif
