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

#include <era/fcbticket.h>
#include <uic9183/uic9183head.h>
#include <uic9183/vendor0080block.h>

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
        if (const auto u_flex = p.findBlock<Fcb::UicRailTicketData>(); u_flex.isValid()) {
            node.setContextDateTime(u_flex.issuingDetail.issueingDateTime());
        } else if (const auto u_head = p.findBlock<Uic9183Head>(); u_head.isValid()) {
            node.setContextDateTime(u_head.issuingDateTime());
        }
    }
}

void Uic9183DocumentProcessor::preExtract(ExtractorDocumentNode &node, [[maybe_unused]] const ExtractorEngine *engine) const
{
    const auto p = node.content<Uic9183Parser>();

    TrainTrip trip, returnTrip;
    Ticket ticket;
    ticket.setName(p.name());
    ticket.setTicketToken(QLatin1String("aztecbin:") + QString::fromLatin1(p.rawData().toBase64()));
    Seat seat;
    if (const auto seatingType = p.seatingType(); !seatingType.isEmpty()) {
        seat.setSeatingType(seatingType);
    }

    const auto rct2 = p.rct2Ticket();
    if (rct2.isValid()) {
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
                Q_FALLTHROUGH();
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
    }

    const auto fcb = p.findBlock<Fcb::UicRailTicketData>();
    if (fcb.isValid() && !fcb.transportDocument.isEmpty()) {
        const auto issueDt = fcb.issuingDetail.issueingDateTime();
        const auto doc = fcb.transportDocument.at(0);
        if (doc.ticket.userType() == qMetaTypeId<Fcb::ReservationData>()) {
            const auto irt = doc.ticket.value<Fcb::ReservationData>();
            trip.setDepartureStation(p.outboundDepartureStation());
            trip.setArrivalStation(p.outboundArrivalStation());
            trip.setDepartureTime(irt.departureDateTime(issueDt));
        } else if (doc.ticket.userType() == qMetaTypeId<Fcb::OpenTicketData>()) {
            const auto nrt = doc.ticket.value<Fcb::OpenTicketData>();
            trip.setDepartureStation(p.outboundDepartureStation());
            trip.setArrivalStation(p.outboundArrivalStation());
            trip.setDepartureDay(nrt.validFrom(issueDt).date());
            // TODO handle nrt.returnIncluded
        }
    }

    ticket.setTicketedSeat(seat);

    // we have enough for a full TrainReservation result
    ExtractorValidator validator;
    validator.setAcceptedTypes<TrainTrip>();
    if (validator.isValidElement(trip)) {
        trip.setProvider(p.issuer());

        QList<QVariant> results;
        TrainReservation res;
        res.setReservationFor(trip);
        res.setReservationNumber(p.pnr());
        res.setReservedTicket(ticket);
        res.setUnderName(p.person());
        results.push_back(res);

        if (validator.isValidElement(returnTrip)) {
            res.setReservationFor(returnTrip);
            results.push_back(res);
        }

        node.addResult(results);
        return;
    }

    // only Ticket
    ticket.setIssuedBy(p.issuer());
    ticket.setTicketNumber(p.pnr());
    ticket.setUnderName(p.person());
    ticket.setValidFrom(p.validFrom());
    ticket.setValidUntil(p.validUntil());
    node.addResult(QList<QVariant>({ticket}));
}

