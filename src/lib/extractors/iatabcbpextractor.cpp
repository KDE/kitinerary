/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "iatabcbpextractor.h"
#include <iata/iatabcbpparser.h>

#include <KItinerary/ExtractorDocumentNode>
#include <KItinerary/ExtractorResult>

using namespace KItinerary;

QString IataBcbpExtractor::name() const
{
    return QStringLiteral("<IATA BCBP>");
}

bool IataBcbpExtractor::canHandle(const ExtractorDocumentNode &node) const
{
    return node.isA<QString>() && IataBcbpParser::maybeIataBcbp(node.content<QString>());
}

ExtractorResult IataBcbpExtractor::extract(const ExtractorDocumentNode &node, [[maybe_unused]] const ExtractorEngine *engine) const
{
    return IataBcbpParser::parse(node.content<QString>(), node.contextDateTime().date());
}
