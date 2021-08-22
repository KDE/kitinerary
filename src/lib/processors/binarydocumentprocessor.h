/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KItinerary/ExtractorDocumentProcessor>

namespace KItinerary {

/** Processor for generic binary content. */
class BinaryDocumentProcessor : public ExtractorDocumentProcessor
{
public:
    ExtractorDocumentNode createNodeFromData(const QByteArray &encodedData) const override;
    bool matches(const ExtractorFilter &filter, const ExtractorDocumentNode &node) const override;
    QJSValue contentToScriptValue(const ExtractorDocumentNode &node, QJSEngine *engine) const override;
};

}

