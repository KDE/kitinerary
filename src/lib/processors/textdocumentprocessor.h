/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_TEXTDOCUMENTPROCESSOR_H
#define KITINERARY_TEXTDOCUMENTPROCESSOR_H

#include <KItinerary/ExtractorDocumentProcessor>

namespace KItinerary {

/** Processor for plain text documents. */
class TextDocumentProcessor : public ExtractorDocumentProcessor
{
public:
    bool canHandleData(const QByteArray &encodedData, QStringView fileName) const override;
    ExtractorDocumentNode createNodeFromData(const QByteArray &encodedData) const override;
    bool matches(const ExtractorFilter &filter, const ExtractorDocumentNode &node) const override;
};

}

#endif // KITINERARY_TEXTDOCUMENTPROCESSOR_H