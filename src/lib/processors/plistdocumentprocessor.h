/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_PLISTDOCUMENTPROCESSOR_H
#define KITINERARY_PLISTDOCUMENTPROCESSOR_H

#include <KItinerary/ExtractorDocumentProcessor>

namespace KItinerary {

/** Document node processor for Apple property lists. */
class PListDocumentProcessor : public ExtractorDocumentProcessor
{
public:
    bool canHandleData(const QByteArray &encodedData, QStringView fileName) const override;
    ExtractorDocumentNode createNodeFromData(const QByteArray &encodedData) const override;
    void expandNode(ExtractorDocumentNode &node, const ExtractorEngine *engine) const override;
};

}

#endif // KITINERARY_PLISTDOCUMENTPROCESSOR_H
