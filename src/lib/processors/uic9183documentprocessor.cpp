/*
    SPDX-FileCopyrightText: 2018-2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "uic9183documentprocessor.h"

#include <KItinerary/ExtractorResult>
#include <KItinerary/ExtractorValidator>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/Rct2Ticket>
#include <KItinerary/Reservation>
#include <KItinerary/Uic9183Parser>
#include <KItinerary/Uic9183TicketLayout>
#include <KItinerary/Ticket>
#include <KItinerary/TrainTrip>

#include "era/fcbextractor_p.h"
#include "era/fcbticket.h"
#include "era/fcbutil.h"

#include "uic9183/uic9183flex.h"
#include "uic9183/uic9183head.h"
#include "uic9183/vendor0080block.h"

#include "variantvisitor_p.h"

#include <KLocalizedString>

#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>

using namespace KItinerary;

Uic9183DocumentProcessor::Uic9183DocumentProcessor()
{
    qRegisterMetaType<KItinerary::Uic9183TicketLayoutField>();
    qRegisterMetaType<KItinerary::Vendor0080BLOrderBlock>();
}

bool Uic9183DocumentProcessor::canHandleData(const QByteArray &encodedData, [[maybe_unused]] QStringView fileName) const
{
    return Uic9183Parser::maybeUic9183(encodedData);
}

ExtractorDocumentNode Uic9183DocumentProcessor::createNodeFromData(const QByteArray &encodedData) const
{
    Uic9183Parser p;
    p.parse(encodedData);
    if (!p.isValid()) {
        return {};
    }

    ExtractorDocumentNode node;
    node.setContent(p);
    return node;
}

void Uic9183DocumentProcessor::expandNode(ExtractorDocumentNode &node, [[maybe_unused]] const ExtractorEngine *engine) const
{
    // only use the U_HEAD issuing time as context if we have nothing better
    // while that is usually correct it cannot contain a time zone, unlike the (often) enclosing PDF document´
    if (!node.contextDateTime().isValid()) {
        const auto p = node.content<Uic9183Parser>();
        if (const auto u_flex = p.findBlock<Uic9183Flex>(); u_flex.isValid()) {
            node.setContextDateTime(std::visit([](auto &&fcb) { return fcb.issuingDetail.issueingDateTime(); }, u_flex.fcb()));
        } else if (const auto u_head = p.findBlock<Uic9183Head>(); u_head.isValid()) {
            node.setContextDateTime(u_head.issuingDateTime());
        }
    }
}

template <typename CardReferenceTypeT>
static ProgramMembership extractCustomerCard(const CardReferenceTypeT &card)
{
    ProgramMembership p;
    p.setProgramName(card.cardName);
    if (card.cardIdNumIsSet()) {
        p.setMembershipNumber(QString::number(card.cardIdNum));
    } else if (card.cardIdIA5IsSet()) {
        p.setMembershipNumber(QString::fromUtf8(card.cardIdIA5));
    }
    return p;
}

template <typename TariffTypeT>
static ProgramMembership extractCustomerCard(const QList <TariffTypeT> &tariffs)
{
    // TODO what do we do with the (so far theoretical) case of multiple discount cards in use?
    for (const auto &tariff : tariffs) {
        for (const auto &card : tariff.reductionCard) {
            return extractCustomerCard(card);
        }
    }

    return {};
}

void Uic9183DocumentProcessor::preExtract(ExtractorDocumentNode &node, [[maybe_unused]] const ExtractorEngine *engine) const
{
    const auto p = node.content<Uic9183Parser>();

    Ticket ticket;
    ticket.setName(p.name());
    ticket.setTicketToken(QLatin1StringView("aztecbin:") +
                          QString::fromLatin1(p.rawData().toBase64()));
    Seat seat;
    if (const auto seatingType = p.seatingType(); !seatingType.isEmpty()) {
        seat.setSeatingType(seatingType);
    }

    TrainReservation res;
    res.setReservationNumber(p.pnr());
    res.setUnderName(p.person());

    ExtractorValidator validator;
    validator.setAcceptedTypes<TrainTrip>();

    QList<QVariant> results;

    const auto rct2 = p.rct2Ticket();
    if (rct2.isValid()) {
        TrainTrip trip, returnTrip;
        trip.setProvider(p.issuer());

        switch (rct2.type()) {
            case Rct2Ticket::Unknown:
            case Rct2Ticket::RailPass:
                break;
            case Rct2Ticket::Reservation:
            case Rct2Ticket::TransportReservation:
            {
                trip.setTrainNumber(rct2.trainNumber());
                seat.setSeatSection(rct2.coachNumber());
                seat.setSeatNumber(rct2.seatNumber());
                [[fallthrough]];
            }
            case Rct2Ticket::Transport:
            case Rct2Ticket::Upgrade:
            {
                trip.setDepartureStation(p.outboundDepartureStation());
                trip.setArrivalStation(p.outboundArrivalStation());

                if (rct2.outboundDepartureTime().isValid()) {
                    trip.setDepartureDay(rct2.outboundDepartureTime().date());
                } else {
                    trip.setDepartureDay(rct2.firstDayOfValidity());
                }

                if (rct2.outboundDepartureTime() != rct2.outboundArrivalTime()) {
                    trip.setDepartureTime(rct2.outboundDepartureTime());
                    trip.setArrivalTime(rct2.outboundArrivalTime());
                }

                if (rct2.type() == Rct2Ticket::Transport && !p.returnDepartureStation().name().isEmpty()) {
                    returnTrip.setProvider(p.issuer());
                    returnTrip.setDepartureStation(p.returnDepartureStation());
                    returnTrip.setArrivalStation(p.returnArrivalStation());

                    if (rct2.returnDepartureTime().isValid()) {
                        returnTrip.setDepartureDay(rct2.returnDepartureTime().date());
                    } else {
                        returnTrip.setDepartureDay(rct2.firstDayOfValidity());
                    }

                    if (rct2.returnDepartureTime() != rct2.returnArrivalTime()) {
                        returnTrip.setDepartureTime(rct2.returnDepartureTime());
                        returnTrip.setArrivalTime(rct2.returnArrivalTime());
                    }
                }

                break;
            }
        }

        if (const auto currency = rct2.currency(); !currency.isEmpty()) {
            res.setPriceCurrency(currency);
            res.setTotalPrice(rct2.price());
        }

        // provide names for typically "addon" tickets, so we can distinguish them in the UI
        switch (rct2.type()) {
            case Rct2Ticket::Reservation:
                ticket.setName(i18n("Reservation"));
                break;
            case Rct2Ticket::Upgrade:
                ticket.setName(i18n("Upgrade"));
                break;
            default:
                break;
        }

        ticket.setTicketedSeat(seat);
        if (validator.isValidElement(trip)) {
            res.setReservationFor(trip);
            res.setReservedTicket(ticket);
            results.push_back(res);
        }
        if (validator.isValidElement(returnTrip)) {
            res.setReservationFor(returnTrip);
            res.setReservedTicket(ticket);
            results.push_back(res);
        }
    }

    ticket.setValidFrom(p.validFrom());
    ticket.setValidUntil(p.validUntil());
    if (const auto flex = p.findBlock<Uic9183Flex>(); flex.isValid()) {
        res.setPriceCurrency(QString::fromUtf8(std::visit([](auto &&fcb) { return fcb.issuingDetail.currency; }, flex.fcb())));
        for (const auto &doc : flex.transportDocuments()) {
            ticket.setIssuedBy(p.issuer());
            ticket.setTicketNumber(p.pnr());
            FcbExtractor::extractReservation(doc, flex.fcb(), ticket, results);
            FcbExtractor::extractOpenTicket(doc, flex.fcb(), ticket, results);
            FcbExtractor::extractCustomerCard(doc, flex.fcb(), ticket, results);
        }

        FcbExtractor::applyPrice(ticket, flex.fcb());
    }

    if (!results.isEmpty()) {
        node.addResult(results);
        return;
    }

    // only Ticket
    ticket.setTicketedSeat(seat);
    ticket.setIssuedBy(p.issuer());
    ticket.setTicketNumber(p.pnr());
    ticket.setUnderName(p.person());
    if (const auto currency = rct2.currency(); !currency.isEmpty()) {
        ticket.setPriceCurrency(currency);
        ticket.setTotalPrice(rct2.price());
    }
    node.addResult(QList<QVariant>({ticket}));
}

