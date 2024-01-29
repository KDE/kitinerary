/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "plistdocumentprocessor.h"

#include <plist/plistreader_p.h>

#include <KItinerary/ExtractorEngine>
#include <KItinerary/ExtractorDocumentNodeFactory>

#include <QJsonObject>

using namespace KItinerary;

bool PListDocumentProcessor::canHandleData(const QByteArray &encodedData, [[maybe_unused]] QStringView fileName) const
{
    return PListReader::maybePList(encodedData);
}

ExtractorDocumentNode PListDocumentProcessor::createNodeFromData(const QByteArray &encodedData) const
{
    ExtractorDocumentNode node;
    node.setContent(QVariant::fromValue(PListReader(encodedData)));
    return node;
}

static void searchSchemaOrgRecursive(const QJsonValue &val, QJsonArray &result)
{
    if (val.isObject()) {
        const auto obj = val.toObject();
        if (obj.contains(QLatin1StringView("@type"))) {
          result.push_back(obj);
          return;
        }

        for (auto it = obj.begin(); it != obj.end(); ++it) {
            if (it.value().isObject() || it.value().isArray()) {
                searchSchemaOrgRecursive(it.value(), result);
            }
        }
    }

    if (val.isArray()) {
        const auto a = val.toArray();
        for (const auto &v : a) {
            if (v.isObject() || v.isArray()) {
                searchSchemaOrgRecursive(v, result);
            }
        }
    }
}

void PListDocumentProcessor::expandNode(ExtractorDocumentNode &node, const ExtractorEngine *engine) const
{
    const auto plist = node.content<PListReader>();
    const auto nsKeyedArchive = plist.unpackKeyedArchive();
    if (!nsKeyedArchive.isObject()) {
        return;
    }

    // search for schema.org JSON-LD sub-trees in this
    QJsonArray childData;
    searchSchemaOrgRecursive(nsKeyedArchive, childData);
    if (childData.isEmpty()) {
        auto child = engine->documentNodeFactory()->createNode(QVariant::fromValue(QJsonArray({nsKeyedArchive})), u"application/json");
        node.appendChild(child);
    } else {
        auto child = engine->documentNodeFactory()->createNode(childData, u"application/json");
        node.appendChild(child);
    }
}
