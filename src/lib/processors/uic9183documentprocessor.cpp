/*
    SPDX-FileCopyrightText: 2018-2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "uic9183documentprocessor.h"

#include <KItinerary/ExtractorResult>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/Uic9183Parser>
#include <KItinerary/Rct2Ticket>
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

static QJsonValue makeStation(const QString &name)
{
    if (name.isEmpty()) {
        return {};
    }

    QJsonObject station;
    station.insert(QStringLiteral("@type"), QLatin1String("TrainStation"));
    station.insert(QStringLiteral("name"), name);
    return station;
}

static bool isValidTrip(const QJsonObject &trip)
{
    if (trip.size() <= 1) {
        return false;
    }

    return trip.contains(QLatin1String("departureDay"))
        && trip.value(QLatin1String("departureStation")).isObject()
        && trip.value(QLatin1String("arrivalStation")).isObject();
}

void Uic9183DocumentProcessor::preExtract(ExtractorDocumentNode &node, [[maybe_unused]] const ExtractorEngine *engine) const
{
    const auto p = node.content<Uic9183Parser>();

    QJsonObject trip;
    trip.insert(QLatin1String("@type"), QLatin1String("TrainTrip"));
    QJsonObject returnTrip;
    returnTrip.insert(QLatin1String("@type"), QLatin1String("TrainTrip"));

    QJsonObject seat;
    seat.insert(QLatin1String("@type"), QLatin1String("Seat"));
    if (const auto seatingType = p.seatingType(); !seatingType.isEmpty()) {
        seat.insert(QLatin1String("seatingType"), seatingType);
    }

    const auto rct2 = p.rct2Ticket();
    if (rct2.isValid()) {
        switch (rct2.type()) {
            case Rct2Ticket::Reservation:
            case Rct2Ticket::TransportReservation:
            {
                trip.insert(QStringLiteral("trainNumber"), rct2.trainNumber());
                seat.insert(QStringLiteral("seatSection"), rct2.coachNumber());
                seat.insert(QStringLiteral("seatNumber"), rct2.seatNumber());
                Q_FALLTHROUGH();
            }
            case Rct2Ticket::Transport:
            case Rct2Ticket::Upgrade:
            {
                trip.insert(QStringLiteral("departureStation"), makeStation(rct2.outboundDepartureStation()));
                trip.insert(QStringLiteral("arrivalStation"), makeStation(rct2.outboundArrivalStation()));

                if (rct2.outboundDepartureTime().isValid()) {
                    trip.insert(QStringLiteral("departureDay"),  JsonLdDocument::toJsonValue(rct2.outboundDepartureTime().date()));
                } else {
                    trip.insert(QStringLiteral("departureDay"),  JsonLdDocument::toJsonValue(rct2.firstDayOfValidity()));
                }

                if (rct2.outboundDepartureTime() != rct2.outboundArrivalTime()) {
                    trip.insert(QStringLiteral("departureTime"),  JsonLdDocument::toJsonValue(rct2.outboundDepartureTime()));
                    trip.insert(QStringLiteral("arrivalTime"),  JsonLdDocument::toJsonValue(rct2.outboundArrivalTime()));
                }

                if (rct2.type() == Rct2Ticket::Transport && !rct2.returnDepartureStation().isEmpty()) {
                    returnTrip.insert(QStringLiteral("departureStation"), makeStation(rct2.returnDepartureStation()));
                    returnTrip.insert(QStringLiteral("arrivalStation"), makeStation(rct2.returnArrivalStation()));

                    if (rct2.returnDepartureTime().isValid()) {
                        returnTrip.insert(QStringLiteral("departureDay"),  JsonLdDocument::toJsonValue(rct2.returnDepartureTime().date()));
                    } else {
                        returnTrip.insert(QStringLiteral("departureDay"),  JsonLdDocument::toJsonValue(rct2.firstDayOfValidity()));
                    }

                    if (rct2.returnDepartureTime() != rct2.returnArrivalTime()) {
                        returnTrip.insert(QStringLiteral("departureTime"), JsonLdDocument::toJsonValue(rct2.returnDepartureTime()));
                        returnTrip.insert(QStringLiteral("arrivalTime"), JsonLdDocument::toJsonValue(rct2.returnArrivalTime()));
                    }
                }

                break;
            }
            default:
                break;
        }
    }

    QJsonObject ticket;
    ticket.insert(QStringLiteral("@type"), QLatin1String("Ticket"));
    ticket.insert(QStringLiteral("ticketToken"), QString(QLatin1String("aztecbin:") + QString::fromLatin1(p.rawData().toBase64())));
    ticket.insert(QStringLiteral("ticketedSeat"), seat);
    if (rct2.isValid()) {
        switch (rct2.type()) { // provide names for typically "addon" tickets, so we can distinguish them in the UI
            case Rct2Ticket::Reservation:
                ticket.insert(QStringLiteral("name"), i18n("Reservation"));
                break;
            case Rct2Ticket::Upgrade:
                ticket.insert(QStringLiteral("name"), i18n("Upgrade"));
                break;
            default:
                break;
        }
    }

    // we have enough for a full TrainReservation result
    if (isValidTrip(trip)) {
        trip.insert(QLatin1String("provider"), JsonLdDocument::toJson(p.issuer()));

        QJsonArray results;
        QJsonObject res;
        res.insert(QStringLiteral("@type"), QLatin1String("TrainReservation"));
        res.insert(QStringLiteral("reservationFor"), trip);
        res.insert(QStringLiteral("reservationNumber"), p.pnr());
        res.insert(QStringLiteral("reservedTicket"), ticket);
        res.insert(QStringLiteral("underName"), JsonLdDocument::toJson(p.person()));
        results.push_back(res);

        if (isValidTrip(returnTrip)) {
            res.insert(QStringLiteral("reservationFor"), returnTrip);
            results.push_back(res);
        }

        node.addResult(results);
        return;
    }

    // only Ticket
    ticket.insert(QStringLiteral("name"), p.name());
    ticket.insert(QStringLiteral("issuedBy"), JsonLdDocument::toJson(p.issuer()));
    ticket.insert(QStringLiteral("ticketNumber"), p.pnr());
    ticket.insert(QStringLiteral("underName"), JsonLdDocument::toJson(p.person()));
    if (p.validFrom().isValid()) {
        ticket.insert(QStringLiteral("validFrom"), JsonLdDocument::toJsonValue(p.validFrom()));
    }
    if (p.validUntil().isValid()) {
        ticket.insert(QStringLiteral("validUntil"), JsonLdDocument::toJsonValue(p.validUntil()));
    }
    node.addResult(QJsonArray({ticket}));
}

