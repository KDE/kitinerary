/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_GENERICBOARDINGPASSEXTRACTOR_H
#define KITINERARY_GENERICBOARDINGPASSEXTRACTOR_H

#include <KItinerary/AbstractExtractor>
#include <KItinerary/ExtractorFilter>

namespace KItinerary {

/** Generic PDF boarding pass extractor. */
class GenericBoardingPassExtractor : public AbstractExtractor
{
public:
    GenericBoardingPassExtractor();
    ~GenericBoardingPassExtractor();

    QString name() const override;
    bool canHandle(const KItinerary::ExtractorDocumentNode & node) const override;
    ExtractorResult extract(const ExtractorDocumentNode &node, const ExtractorEngine *engine) const override;

private:
    ExtractorFilter m_filter;
};

}

#endif // KITINERARY_GENERICBOARDINGPASSEXTRACTOR_H
