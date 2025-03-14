/*
    SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_DOSIPASDOCUMENTPROCESSOR_H
#define KITINERARY_DOSIPASDOCUMENTPROCESSOR_H

#include <KItinerary/ExtractorDocumentProcessor>

namespace KItinerary {

/** Document processor for ERA ELB ticket barcodes. */
class DosipasDocumentProcessor : public ExtractorDocumentProcessor
{
public:
    [[nodiscard]] bool canHandleData(const QByteArray &encodedData, QStringView fileName) const override;
    [[nodiscard]] ExtractorDocumentNode createNodeFromData(const QByteArray &encodedData) const override;
    void preExtract(ExtractorDocumentNode &node, const ExtractorEngine *engine) const override;
};

}

#endif
