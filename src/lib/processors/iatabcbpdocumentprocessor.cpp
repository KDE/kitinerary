/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "iatabcbpdocumentprocessor.h"

#include "iata/iatabcbp.h"
#include "iata/iatabcbpparser.h"

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
    node.addResult(IataBcbpParser::parse(bcbp, node.contextDateTime()));
}

bool IataBcbpDocumentProcessor::matches(const ExtractorFilter &filter, const ExtractorDocumentNode &node) const
{
    const auto bcbp = node.content<IataBcbp>();
    const auto ums = bcbp.uniqueMandatorySection();
    const auto ucs = bcbp.uniqueConditionalSection();
    if (matchesGadget(filter, &ums) || matchesGadget(filter, &ucs)) {
        return true;
    }
    const auto legCount = ums.numberOfLegs();
    for (int i = 0; i < legCount; ++i) {
        const auto rms = bcbp.repeatedMandatorySection(i);
        const auto rcs = bcbp.repeatedConditionalSection(i);
        if (matchesGadget(filter, &rms) || matchesGadget(filter, &rcs)) {
            return true;
        }
    }
    return false;
}
