/*
   SPDX-FileCopyrightText: ⓒ 2025 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "pkpassesdocumentprocessor.h"

#include <KItinerary/ExtractorDocumentNode>
#include <KItinerary/ExtractorDocumentNodeFactory>
#include <KItinerary/ExtractorEngine>

#include <KPkPass/Passes>

using namespace Qt::Literals;
using namespace KItinerary;

Q_DECLARE_METATYPE(KItinerary::Internal::OwnedPtr<KPkPass::Passes>)

bool PkPassesDocumentProcessor::canHandleData(const QByteArray &encodedData, QStringView fileName) const
{
    return (encodedData.startsWith("PK\x03\x04") && QByteArrayView(encodedData).left(256).contains(".pkpass")) ||
            fileName.endsWith(QLatin1StringView(".pkpasses"), Qt::CaseInsensitive);
}

ExtractorDocumentNode PkPassesDocumentProcessor::createNodeFromData(const QByteArray &encodedData) const
{
    auto passes = KPkPass::Passes::fromData(encodedData);
    if (!passes) {
        return {};
    }

    ExtractorDocumentNode node;
    node.setContent<Internal::OwnedPtr<KPkPass::Passes>>(passes);
    return node;
}

void PkPassesDocumentProcessor::expandNode(ExtractorDocumentNode &node, const ExtractorEngine *engine) const
{
    const auto passes = node.content<KPkPass::Passes*>();
    const auto entries = passes->entries();
    for (const auto &entry : entries) {
        auto child = engine->documentNodeFactory()->createNode(passes->passData(entry));
        node.appendChild(child);
    }
}

void PkPassesDocumentProcessor::destroyNode(ExtractorDocumentNode &node) const
{
    destroyIfOwned<KPkPass::Passes>(node);
}
