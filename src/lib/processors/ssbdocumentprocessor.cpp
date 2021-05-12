/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ssbdocumentprocessor.h"

#include "era/ssbv1ticket.h"
#include "era/ssbv3ticket.h"

using namespace KItinerary;

bool SsbDocumentProcessor::canHandleData(const QByteArray &encodedData, [[maybe_unused]] QStringView fileName) const
{
    return SSBv3Ticket::maybeSSB(encodedData) || SSBv1Ticket::maybeSSB(encodedData);
}

ExtractorDocumentNode SsbDocumentProcessor::createNodeFromData(const QByteArray &encodedData) const
{
    ExtractorDocumentNode node;
    if (SSBv3Ticket::maybeSSB(encodedData)) {
        auto ticket = SSBv3Ticket(encodedData);
        if (ticket.isValid()) {
            node.setContent(ticket);
        }
    } else if (SSBv1Ticket::maybeSSB(encodedData)) {
        auto ticket = SSBv1Ticket(encodedData);
        if (ticket.isValid()) {
            node.setContent(ticket);
        }
    }
    return node;
}
