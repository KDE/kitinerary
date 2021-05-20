/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "iatabcbpdocumentprocessor.h"

#include <iata/iatabcbp.h>
#include <iata/iatabcbpparser.h>

#include <KItinerary/ExtractorResult>

using namespace KItinerary;

bool IataBcbpDocumentProcessor::canHandleData(const QByteArray &encodedData, [[maybe_unused]] QStringView fileName) const
{
    return IataBcbp::maybeIataBcbp(encodedData);
}

ExtractorDocumentNode IataBcbpDocumentProcessor::createNodeFromData(const QByteArray &encodedData) const
{
    ExtractorDocumentNode node;
    IataBcbp ticket(QString::fromUtf8(encodedData));
    if (ticket.isValid()) {
        node.setContent(ticket);
    }
    return node;
}

void IataBcbpDocumentProcessor::preExtract(ExtractorDocumentNode &node, [[maybe_unused]] const ExtractorEngine *engine) const
{
    const auto bcbp = node.content<IataBcbp>();
    node.addResult(IataBcbpParser::parse(bcbp, node.contextDateTime().date()));
}
