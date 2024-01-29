/*
    SPDX-FileCopyrightText: 2019-2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "vdvdocumentprocessor.h"

#include "asn1/berelement.h"

#include <KItinerary/ExtractorResult>
#include <KItinerary/Ticket>
#include <KItinerary/VdvTicket>
#include <KItinerary/VdvTicketContent>
#include <KItinerary/VdvTicketParser>

#include <KLocalizedString>

#include <QByteArray>

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

    Seat seat;
    switch (vdv.serviceClass()) {
        case VdvTicket::UnknownClass:
            break;
        case VdvTicket::FirstClass:
        case VdvTicket::FirstClassUpgrade:
            seat.setSeatingType(QStringLiteral("1"));
            break;
        case VdvTicket::SecondClass:
            seat.setSeatingType(QStringLiteral("2"));
            break;
        default:
            break;
    }

    Ticket ticket;
    ticket.setTicketToken(QLatin1StringView("aztecbin:") +
                          QString::fromLatin1(vdv.rawData().toBase64()));
    ticket.setTicketedSeat(seat);
    if (vdv.serviceClass() == VdvTicket::FirstClassUpgrade) {
        ticket.setName(i18n("Upgrade"));
    } else if (const auto hdr = vdv.header(); hdr && hdr->productId == 9999) {
      ticket.setName(QLatin1StringView("Deutschlandticket"));
    } else {
        ticket.setName(i18n("Ticket"));
    }
    ticket.setTicketNumber(vdv.ticketNumber());
    ticket.setValidFrom(vdv.beginDateTime());
    ticket.setValidUntil(vdv.endDateTime());
    ticket.setUnderName(vdv.person());

    if (const auto basicData = vdv.productData().find(VdvTicketBasicData::Tag).contentAt<VdvTicketBasicData>(); basicData && basicData->price) {
        ticket.setTotalPrice(basicData->price / 100.0);
        ticket.setPriceCurrency(QStringLiteral("EUR"));
    }

    node.addResult(QList<QVariant>({ticket}));
}
