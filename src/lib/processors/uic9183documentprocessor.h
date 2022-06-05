/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KItinerary/ExtractorDocumentProcessor>

namespace KItinerary {

/** Processor for UIC 918.3 containers. */
class Uic9183DocumentProcessor : public ExtractorDocumentProcessor
{
public:
    explicit Uic9183DocumentProcessor();
    bool canHandleData(const QByteArray &encodedData, QStringView fileName) const override;
    ExtractorDocumentNode createNodeFromData(const QByteArray &encodedData) const override;
    void expandNode(ExtractorDocumentNode &node, const ExtractorEngine *engine) const override;
    void preExtract(ExtractorDocumentNode &node, const ExtractorEngine *engine) const override;
};

}

