/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_ACTIVITYPUBEXTRACTOR_H
#define KITINERARY_ACTIVITYPUBEXTRACTOR_H

#include <KItinerary/AbstractExtractor>
#include <KItinerary/ExtractorFilter>

namespace KItinerary {

/** ActivityPub extractor.
 *  @see https://www.w3.org/TR/activitypub
 *  @see https://www.w3.org/ns/activitystreams
 *  @see https://www.w3.org/TR/activitystreams-core
 *  @see https://www.w3.org/TR/activitystreams-vocabulary
 *  @see https://www.w3.org/TR/json-ld
 *  @see https://www.w3.org/TR/json-ld-api
 */
class ActivityPubExtractor : public AbstractExtractor
{
public:
    explicit ActivityPubExtractor();
    ~ActivityPubExtractor();

    QString name() const override;
    bool canHandle(const KItinerary::ExtractorDocumentNode & node) const override;
    ExtractorResult extract(const ExtractorDocumentNode &node, const ExtractorEngine *engine) const override;
};

}

#endif // KITINERARY_ACTIVITYPUBEXTRACTOR_H
