/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kitinerary_export.h"

class QString;

namespace KItinerary {
class ExtractorDocumentNode;
class ExtractorEngine;
class ExtractorResult;

/** Abstract base class for data extractors. */
class KITINERARY_EXPORT AbstractExtractor
{
public:
    virtual ~AbstractExtractor();

    /** Identifier for this extractor.
     *  Mainly used for diagnostics and tooling.
     */
    virtual QString name() const = 0;

    /** Fast check whether this extractor is applicable for @p node. */
    virtual bool canHandle(const ExtractorDocumentNode &node) const = 0;

    /** Extract data from @p node. */
    virtual ExtractorResult extract(const ExtractorDocumentNode &node, const ExtractorEngine *engine) const = 0;
};

}

