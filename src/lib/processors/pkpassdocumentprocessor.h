/*
   SPDX-FileCopyrightText: 2019-2021 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_PKPASSDOCUMENTPROCESSOR_H
#define KITINERARY_PKPASSDOCUMENTPROCESSOR_H

#include <KItinerary/ExtractorDocumentProcessor>

namespace KItinerary {

/** Processor for Apple Wallet pass files. */
class PkPassDocumentProcessor : public ExtractorDocumentProcessor
{
public:
    bool canHandleData(const QByteArray &encodedData, QStringView fileName) const override;
    ExtractorDocumentNode createNodeFromData(const QByteArray &encodedData) const override;
    ExtractorDocumentNode createNodeFromContent(const QVariant &decodedData) const override;
    void expandNode(ExtractorDocumentNode &node, const ExtractorEngine *engine) const override;
    void preExtract(ExtractorDocumentNode &node, const ExtractorEngine *engine) const override;
    bool matches(const ExtractorFilter &filter, const ExtractorDocumentNode &node) const override;
    QJSValue contentToScriptValue(const ExtractorDocumentNode &node, QJSEngine *engine) const override;
    void postExtract(ExtractorDocumentNode &node) const override;
    void destroyNode(ExtractorDocumentNode &node) const override;
};

}

#endif // KITINERARY_PKPASSDOCUMENTPROCESSOR_H