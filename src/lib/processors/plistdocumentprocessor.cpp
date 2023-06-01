/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "plistdocumentprocessor.h"

#include <plist/plistreader_p.h>

#include <KItinerary/ExtractorEngine>
#include <KItinerary/ExtractorDocumentNodeFactory>

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

void PListDocumentProcessor::expandNode(ExtractorDocumentNode &node, const ExtractorEngine *engine) const
{
    const auto plist = node.content<PListReader>();
    const auto nsKeyedArchive = plist.unpackKeyedArchive();
    if (!nsKeyedArchive.isObject()) {
        return;
    }
    auto child = engine->documentNodeFactory()->createNode(QVariant::fromValue(QJsonArray({nsKeyedArchive})), u"application/json");
    node.appendChild(child);
}
