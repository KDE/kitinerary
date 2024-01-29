/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "httpresponseprocessor.h"

#include "http/httpresponse.h"

#include <KItinerary/ExtractorDocumentNode>
#include <KItinerary/ExtractorDocumentNodeFactory>
#include <KItinerary/ExtractorEngine>

using namespace KItinerary;

void HttpResponseProcessor::expandNode(ExtractorDocumentNode &node, const ExtractorEngine *engine) const
{
    const auto r = node.content<HttpResponse>();

    auto contentNode = engine->documentNodeFactory()->createNode(r.content());
    node.appendChild(contentNode);
}

bool HarDocumentProcessor::canHandleData([[maybe_unused]] const QByteArray &encodedData, QStringView fileName) const
{
  return fileName.endsWith(QLatin1StringView(".har"));
}

ExtractorDocumentNode HarDocumentProcessor::createNodeFromData(const QByteArray &encodedData) const
{
    ExtractorDocumentNode node;
    node.setContent(encodedData);
    return node;
}

void HarDocumentProcessor::expandNode(ExtractorDocumentNode &node, const ExtractorEngine *engine) const
{
    const auto httpResponses = HttpResponse::fromHarFile(node.content<QByteArray>());
    for (const auto &httpResponse : httpResponses) {
        auto child = engine->documentNodeFactory()->createNode(httpResponse, u"internal/http-response");
        child.setContextDateTime(httpResponse.requestDateTime());
        node.appendChild(child);
    }
}
