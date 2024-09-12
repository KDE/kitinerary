/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "activitypubextractor.h"

#include "json/jsonldfilterengine.h"

#include <KItinerary/ExtractorDocumentNode>
#include <KItinerary/ExtractorResult>

#include <QJsonArray>
#include <QJsonObject>

using namespace KItinerary;

ActivityPubExtractor::ActivityPubExtractor() = default;
ActivityPubExtractor::~ActivityPubExtractor() = default;

QString ActivityPubExtractor::name() const
{
    return QStringLiteral("<ActivityPub>");
}

static bool isActivityStreamsContext(const QJsonValue &context)
{
  return context.isString() &&
         context.toString() ==
             QLatin1StringView("https://www.w3.org/ns/activitystreams");
}

bool ActivityPubExtractor::canHandle(const ExtractorDocumentNode &node) const
{
    const auto array = node.content<QJsonArray>();
    for (const auto &v : array) {
        if (!v.isObject()) {
            continue;
        }
        const auto obj = v.toObject();
        const auto context = obj.value(QLatin1StringView("@context"));
        if (isActivityStreamsContext(context)) {
            return true;
        }
        if (context.isArray()) {
            const auto contexts = context.toArray();
            if (std::any_of(contexts.begin(), contexts.end(), isActivityStreamsContext)) {
                return true;
            }
        }
    }
    return false;
}

static void convertPlace(QJsonObject &obj)
{
  QJsonObject geo({
      {QLatin1StringView("@type"), QLatin1StringView("GeoCoordinates")},
      {QLatin1StringView("latitude"), obj.value(QLatin1StringView("latitude"))},
      {QLatin1StringView("longitude"), obj.value(QLatin1StringView("longitude"))},
  });
  obj.insert(QLatin1StringView("geo"), geo);
}

// filter functions applied to objects of the corresponding (already normalized) type
// IMPORTANT: keep alphabetically sorted by type!
static constexpr const JsonLdFilterEngine::TypeFilter type_filters[] = {
    { "Place", convertPlace },
};

// property mappings
// IMPORTANT: keep alphabetically sorted by type!
static constexpr const JsonLdFilterEngine::PropertyMapping property_mappings[] = {
    { "Event", "endTime", "endDate" },
    { "Event", "startTime", "startDate"},
};

// in theory we would need the whole JSON-LD schema-aware normalization here
// in practice this is good enough and works
// (should we ever need more, KHealthCertificate has some of that JSON-LD code)
static QJsonValue convertActivityStreamObject(const QJsonValue &value)
{
    if (!value.isObject()) {
        return value;
    }

    auto obj = value.toObject();
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        if (it.value().isObject()) {
            (*it) = convertActivityStreamObject(it.value());
        }
    }

    if (const auto t = obj.value(QLatin1StringView("type")).toString();
        !t.isEmpty()) {
      obj.insert(QLatin1StringView("@type"), t);
    }

    JsonLdFilterEngine filterEngine;
    filterEngine.setTypeFilters(type_filters);
    filterEngine.setPropertyMappings(property_mappings);
    filterEngine.filterRecursive(obj);

    return obj;
}

ExtractorResult ActivityPubExtractor::extract(const ExtractorDocumentNode &node, [[maybe_unused]] const ExtractorEngine *engine) const
{
    const auto array = node.content<QJsonArray>();
    QJsonArray result;
    std::transform(array.begin(), array.end(), std::back_inserter(result), convertActivityStreamObject);
    return result;
}
