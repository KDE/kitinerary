/*
   SPDX-FileCopyrightText: 2017-2021 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "jsonlddocumentprocessor.h"

#include <KItinerary/ExtractorResult>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

using namespace KItinerary;

static bool contentStartsWith(const QByteArray &data, char s)
{
    for (const auto c : data) {
        if (std::isspace(c)) {
            continue;
        }
        return c == s;
    }
    return false;
}

bool JsonLdDocumentProcessor::canHandleData(const QByteArray &encodedData, QStringView fileName) const
{
    return contentStartsWith(encodedData, '[')
        || contentStartsWith(encodedData, '{')
        || fileName.endsWith(QLatin1String(".json"), Qt::CaseInsensitive)
        || fileName.endsWith(QLatin1String(".jsonld"), Qt::CaseInsensitive);
}

ExtractorDocumentNode JsonLdDocumentProcessor::createNodeFromData(const QByteArray &encodedData) const
{
    QJsonParseError error;
    const auto doc = QJsonDocument::fromJson(encodedData, &error);
    if (error.error != QJsonParseError::NoError) {
        return {};
    }

    QJsonArray data;
    if (doc.isObject()) {
        data.push_back(doc.object());
    } else if (doc.isArray()) {
        data = doc.array();
    } else {
        return {};
    }

    ExtractorDocumentNode node;
    node.setContent(data);
    return node;
}

void JsonLdDocumentProcessor::preExtract(ExtractorDocumentNode &node, [[maybe_unused]] const ExtractorEngine *engine) const
{
    // pass through JSON data, so the using code can apply post-processing to that
    node.addResult(node.content<QJsonArray>());
}
