/*
    SPDX-FileCopyrightText: 2019-2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "vdvdocumentprocessor.h"

#include <KItinerary/ExtractorResult>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/VdvTicket>
#include <KItinerary/VdvTicketParser>

#include <KLocalizedString>

#include <QByteArray>
#include <QJsonArray>
#include <QJsonObject>

using namespace KItinerary;

bool VdvDocumentProcessor::canHandleData(const QByteArray &encodedData, [[maybe_unused]] QStringView fileName) const
{
    return VdvTicketParser::maybeVdvTicket(encodedData);
}

ExtractorDocumentNode VdvDocumentProcessor::createNodeFromData(const QByteArray &encodedData) const
{
    VdvTicketParser p;
    if (!p.parse(encodedData)) {
        return {};
    }

    ExtractorDocumentNode node;
    node.setContent(p.ticket());
    return node;
}

void VdvDocumentProcessor::preExtract(ExtractorDocumentNode &node, [[maybe_unused]] const ExtractorEngine *engine) const
{
    const auto vdv = node.content<VdvTicket>();

    QJsonObject seat;
    seat.insert(QStringLiteral("@type"), QLatin1String("Seat"));
    switch (vdv.serviceClass()) {
        case VdvTicket::FirstClass:
        case VdvTicket::FirstClassUpgrade:
            seat.insert(QStringLiteral("seatingType"), QStringLiteral("1"));
            break;
        case VdvTicket::SecondClass:
            seat.insert(QStringLiteral("seatingType"), QStringLiteral("2"));
            break;
        default:
            break;
    }

    QJsonObject ticket;
    ticket.insert(QStringLiteral("@type"), QLatin1String("Ticket"));
    ticket.insert(QStringLiteral("ticketToken"), QString(QLatin1String("aztecbin:") + QString::fromLatin1(vdv.rawData().toBase64())));
    ticket.insert(QStringLiteral("ticketedSeat"), seat);
    if (vdv.serviceClass() == VdvTicket::FirstClassUpgrade) {
        ticket.insert(QStringLiteral("name"), i18n("Upgrade"));
    } else {
        ticket.insert(QStringLiteral("name"), i18n("Ticket"));
    }
    ticket.insert(QStringLiteral("ticketNumber"), vdv.ticketNumber());
    ticket.insert(QStringLiteral("validFrom"), JsonLdDocument::toJsonValue(vdv.beginDateTime()));
    ticket.insert(QStringLiteral("validUntil"), JsonLdDocument::toJsonValue(vdv.endDateTime()));
    ticket.insert(QStringLiteral("underName"), JsonLdDocument::toJson(vdv.person()));
    node.addResult(QJsonArray({ticket}));
}
