/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_SSBDOCUMENTPROCESSOR_H
#define KITINERARY_SSBDOCUMENTPROCESSOR_H

#include <KItinerary/ExtractorDocumentProcessor>

namespace KItinerary {

/** Document processor for ERA SSB ticket barcodes. */
class SsbDocumentProcessor : public ExtractorDocumentProcessor
{
public:
    bool canHandleData(const QByteArray &encodedData, QStringView fileName) const override;
    ExtractorDocumentNode createNodeFromData(const QByteArray &encodedData) const override;
};

}

#endif // KITINERARY_SSBDOCUMENTPROCESSOR_H
