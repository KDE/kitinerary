/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "eradocumentprocessor.h"

#include "era/elbticket.h"
#include "era/ssbticketreader.h"
#include "era/ssbv1ticket.h"
#include "era/ssbv2ticket.h"
#include "era/ssbv3ticket.h"

using namespace KItinerary;

bool ElbDocumentProcessor::canHandleData(const QByteArray &encodedData, [[maybe_unused]] QStringView fileName) const
{
    return ELBTicket::maybeELBTicket(encodedData);
}

ExtractorDocumentNode ElbDocumentProcessor::createNodeFromData(const QByteArray &encodedData) const
{
    ExtractorDocumentNode node;
    if (const auto ticket = ELBTicket::parse(encodedData)) {
        node.setContent(*ticket);
    }
    return node;
}

bool SsbDocumentProcessor::canHandleData(const QByteArray &encodedData, [[maybe_unused]] QStringView fileName) const
{
    return SSBv3Ticket::maybeSSB(encodedData) || SSBv2Ticket::maybeSSB(encodedData) || SSBv1Ticket::maybeSSB(encodedData);
}

ExtractorDocumentNode SsbDocumentProcessor::createNodeFromData(const QByteArray &encodedData) const
{
    ExtractorDocumentNode node;
    node.setContent(SSBTicketReader::read(encodedData));
    return node;
}
