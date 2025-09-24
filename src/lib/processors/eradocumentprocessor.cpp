/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "eradocumentprocessor.h"
#include "logging.h"

#include "era/elbticket.h"
#include "era/ssbticketreader.h"
#include "era/ssbv1ticket.h"
#include "era/ssbv2ticket.h"
#include "era/ssbv3ticket.h"

#include "text/pricefinder_p.h"

#include "uic9183/uicstationcode_p.h"

#include <KItinerary/ExtractorResult>

#include <KItinerary/Reservation>
#include <KItinerary/Ticket>
#include <KItinerary/TrainTrip>

using namespace Qt::Literals;
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
    return SSBTicketReader::maybeSSB(encodedData);
}

ExtractorDocumentNode SsbDocumentProcessor::createNodeFromData(const QByteArray &encodedData) const
{
    ExtractorDocumentNode node;
    node.setContent(SSBTicketReader::read(encodedData));
    return node;
}

[[nodiscard]] static TrainStation makeStation(int idType, const QString &alphaId, int numericId)
{
    TrainStation station;
    if (idType == 0 && numericId > 10'00000 && numericId < 99'99999) {
        station.setIdentifier("uic:"_L1 + QString::number(numericId));
        station.setName(QString::number(numericId));
    } else if (idType == 0 && numericId > 10'00000'0 && numericId < 99'99999'9 && UicStationCode::checksumDigit(numericId / 10) == numericId % 10) {
        station.setIdentifier("uic:"_L1 + QString::number(numericId / 10));
        station.setName(QString::number(numericId / 10));
    } else if (idType == 1 && alphaId.size() == 5 && std::ranges::all_of(alphaId, [](QChar c) { return c.isUpper(); })) {
        // TODO is the identifier type defined in that case??
        station.setName(alphaId);
    }
    return station;
}

void SsbDocumentProcessor::preExtract(ExtractorDocumentNode &node, [[maybe_unused]] const ExtractorEngine *engine) const
{
    if (node.isA<SSBv3Ticket>()) {
        const auto ssb = node.content<SSBv3Ticket>();

        Seat seat;
        seat.setSeatingType(QString::number(ssb.classOfTravel()));
        Ticket ticket;
        ticket.setTicketToken("aztecbin:"_L1 + QString::fromLatin1(ssb.rawData().toBase64()));

        Organization issuer;
        issuer.setIdentifier("uic:"_L1 + QString::number(ssb.issuerCode())); // left pad with 0 to 4 digets?
        TrainTrip trip;
        trip.setProvider(issuer);

        TrainReservation res;
        res.setReservationNumber(ssb.tcn());
        ticket.setTicketNumber(ssb.tcn());

        switch (ssb.ticketTypeCode()) {
            case SSBv3Ticket::IRT_RES_BOA:
                trip.setDepartureDay(ssb.type1DepartureDay(node.contextDateTime()));
                trip.setTrainNumber(ssb.type1TrainNumber());
                seat.setSeatSection(QString::number(ssb.type1CoachNumber()));
                seat.setSeatNumber(ssb.type1SeatNumber());
                trip.setDepartureStation(makeStation(ssb.type1StationCodeNumericOrAlpha(), ssb.type1DepartureStationAlpha(), ssb.type1DepartureStationNum()));
                trip.setDepartureStation(makeStation(ssb.type1StationCodeNumericOrAlpha(), ssb.type1ArrivalStationAlpha(), ssb.type1ArrivalStationNum()));
                break;
            case SSBv3Ticket::NRT:
                trip.setDepartureStation(makeStation(ssb.type2StationCodeNumericOrAlpha(), ssb.type2DepartureStationAlpha(), ssb.type2DepartureStationNum()));
                trip.setArrivalStation(makeStation(ssb.type2StationCodeNumericOrAlpha(), ssb.type2ArrivalStationAlpha(), ssb.type2ArrivalStationNum()));
                ticket.setValidFrom(ssb.type2ValidFrom(node.contextDateTime()).startOfDay());
                ticket.setValidUntil({ssb.type2ValidUntil(node.contextDateTime()), {23, 59, 59}});
                if (const auto &t = ssb.type2OpenText().trimmed(); !t.isEmpty()) {
                    ticket.setName(t);
                }
                trip.setDepartureDay(ssb.type2ValidFrom(node.contextDateTime()));
                break;
            case SSBv3Ticket::GRT:
            {
                trip.setDepartureStation(makeStation(ssb.type3StationCodeNumericOrAlpha(), ssb.type3DepartureStationAlpha(), ssb.type3DepartureStationNum()));
                trip.setArrivalStation(makeStation(ssb.type3StationCodeNumericOrAlpha(), ssb.type3ArrivalStationAlpha(), ssb.type3ArrivalStationNum()));
                ticket.setValidFrom(ssb.type3ValidFrom(node.contextDateTime()).startOfDay());
                ticket.setValidUntil({ssb.type3ValidUntil(node.contextDateTime()), {23, 59, 59}});
                trip.setDepartureDay(ssb.type3ValidFrom(node.contextDateTime()));
                Person p;
                p.setName(ssb.type3NameOfGroupLeader());
                res.setUnderName(p);
                break;
            }
            case SSBv3Ticket::RPT:
                qCWarning(Log) << "Unsupported SSB v3 ticket type:" << ssb.ticketTypeCode();
                return;
        }

        // e.g. MAV has the price in openText
        PriceFinder finder;
        if (const auto &price = finder.findHighest(ticket.name()); price.hasResult()) {
            ticket.setName(QString());
            res.setPriceCurrency(price.currency);
            res.setTotalPrice(price.value);
        }

        // turn incomplete things into generic tickets (e.g. platform keybards)
        if (trip.departureStation().name().isEmpty() || trip.arrivalStation().name().isEmpty()) {
            ticket.setIssuedBy(issuer);
            node.addResult(QList<QVariant>{QVariant::fromValue(ticket)});
            return;
        }

        res.setReservationFor(trip);
        ticket.setTicketedSeat(seat);
        res.setReservedTicket(ticket);
        node.addResult(QList<QVariant>{QVariant::fromValue(res)});
    }
}
