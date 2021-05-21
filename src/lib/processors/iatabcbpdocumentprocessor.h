/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_IATABCBPDOCUMENTPROCESSOR_H
#define KITINERARY_IATABCBPDOCUMENTPROCESSOR_H

#include <KItinerary/ExtractorDocumentProcessor>

namespace KItinerary {

/** Document node processor for IATA BCBPs. */
class IataBcbpDocumentProcessor : public ExtractorDocumentProcessor
{
public:
    bool canHandleData(const QByteArray &encodedData, QStringView fileName) const override;
    ExtractorDocumentNode createNodeFromData(const QByteArray &encodedData) const override;
    void preExtract(ExtractorDocumentNode &node, const ExtractorEngine *engine) const override;
    bool matches(const ExtractorFilter &filter, const ExtractorDocumentNode &node) const override;
};

}

#endif // KITINERARY_IATABCBPDOCUMENTPROCESSOR_H
