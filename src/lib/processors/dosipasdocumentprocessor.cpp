/*
    SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "dosipasdocumentprocessor.h"

#include "variantvisitor_p.h"

#include "era/dosipas1.h"
#include "era/dosipas2.h"
#include "era/fcbextractor_p.h"

#include <KItinerary/ExtractorResult>
#include <KItinerary/Ticket>

using namespace Qt::Literals;
using namespace KItinerary;

bool DosipasDocumentProcessor::canHandleData(const QByteArray &encodedData, [[maybe_unused]] QStringView fileName) const
{
    return encodedData.startsWith("\x01Uc") || encodedData.startsWith("\x01Ue")
        || encodedData.startsWith("\x81Uc") || encodedData.startsWith("\x81Ue");
}

ExtractorDocumentNode DosipasDocumentProcessor::createNodeFromData(const QByteArray &encodedData) const
{
    ExtractorDocumentNode node;
    if (auto container = Dosipas::v2::UicBarcodeHeader(encodedData); container.isValid()) {
        node.setContent(container);
    } else if (auto container = Dosipas::v1::UicBarcodeHeader(encodedData); container.isValid()) {
        node.setContent(container);
    }
    return node;
}

void DosipasDocumentProcessor::preExtract(ExtractorDocumentNode &node, [[maybe_unused]] const ExtractorEngine *engine) const
{
    Ticket ticket;
    const auto rawData = VariantVisitor([](auto &&dosipas) {
        return dosipas.rawData();
    }).visit<Dosipas::v1::UicBarcodeHeader, Dosipas::v2::UicBarcodeHeader>(node.content());
    ticket.setTicketToken("aztecbin:"_L1 + QString::fromLatin1(rawData.toBase64()));

    const auto fcb = VariantVisitor([](auto &&dosipas) -> std::optional<Fcb::UicRailTicketData> {
        for (const auto &payload : dosipas.level2SignedData.level1Data.dataSequence) {
            if (auto fcb = payload.fcb(); fcb) {
                return fcb;
            }
        }
        return std::nullopt;
    }).visit<Dosipas::v1::UicBarcodeHeader, Dosipas::v2::UicBarcodeHeader>(node.content());

    if (!fcb) {
        node.addResult(QList<QVariant>({ticket}));
        return;
    }

    ticket.setName(FcbExtractor::ticketName(*fcb));
    Seat seat;
    seat.setSeatingType(FcbExtractor::seatingType(*fcb));
    ticket.setTicketedSeat(seat);
    ticket.setIssuedBy(FcbExtractor::issuer(*fcb));
    ticket.setTicketNumber(FcbExtractor::pnr(*fcb));
    ticket.setUnderName(FcbExtractor::person(*fcb));
    ticket.setValidFrom(FcbExtractor::validFrom(*fcb));
    ticket.setValidUntil(FcbExtractor::validUntil(*fcb));

    node.addResult(QList<QVariant>({ticket}));
}
