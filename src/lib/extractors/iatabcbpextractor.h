/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KItinerary/AbstractExtractor>

namespace KItinerary {

/** IATA boarding pass barcode extractor. */
class IataBcbpExtractor : public AbstractExtractor
{
public:
    QString name() const override;
    bool canHandle(const ExtractorDocumentNode &node) const override;
    ExtractorResult extract(const ExtractorDocumentNode &node, const ExtractorEngine *engine) const override;
};

}

