/*
    SPDX-FileCopyrightText: 2018-2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "uic9183documentprocessor.h"

#include <KItinerary/ExtractorResult>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/Uic9183Parser>
#include <KItinerary/Rct2Ticket>
#include <uic9183/uic9183head.h>

#include <KLocalizedString>

#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>

using namespace KItinerary;

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
        const auto u_head = p.findBlock<Uic9183Head>();
        node.setContextDateTime(u_head.issuingDateTime());
    }
}

void Uic9183DocumentProcessor::preExtract(ExtractorDocumentNode &node, [[maybe_unused]] const ExtractorEngine *engine) const
{
    const auto p = node.content<Uic9183Parser>();

    QJsonObject trip;
    trip.insert(QStringLiteral("@type"), QLatin1String("TrainTrip"));
    QJsonObject seat;
    seat.insert(QStringLiteral("@type"), QLatin1String("Seat"));
    seat.insert(QStringLiteral("seatingType"), p.seatingType());

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
                QJsonObject dep;
                dep.insert(QStringLiteral("@type"), QLatin1String("TrainStation"));
                dep.insert(QStringLiteral("name"), rct2.outboundDepartureStation());
                trip.insert(QStringLiteral("departureStation"), dep);

                QJsonObject arr;
                arr.insert(QStringLiteral("@type"), QLatin1String("TrainStation"));
                arr.insert(QStringLiteral("name"), rct2.outboundArrivalStation());
                trip.insert(QStringLiteral("arrivalStation"), arr);

                if (rct2.outboundDepartureTime().isValid()) {
                    trip.insert(QStringLiteral("departureDay"), rct2.outboundDepartureTime().date().toString(Qt::ISODate));
                } else {
                    trip.insert(QStringLiteral("departureDay"), rct2.firstDayOfValidity().toString(Qt::ISODate));
                }

                if (rct2.outboundDepartureTime() != rct2.outboundArrivalTime()) {
                    trip.insert(QStringLiteral("departureTime"), rct2.outboundDepartureTime().toString(Qt::ISODate));
                    trip.insert(QStringLiteral("arrivalTime"), rct2.outboundArrivalTime().toString(Qt::ISODate));
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

    QJsonObject res;
    res.insert(QStringLiteral("@type"), QLatin1String("TrainReservation"));
    if (trip.size() > 1) {
        res.insert(QStringLiteral("reservationFor"), trip);
    }
    res.insert(QStringLiteral("reservationNumber"), p.pnr());
    res.insert(QStringLiteral("reservedTicket"), ticket);
    res.insert(QStringLiteral("underName"), JsonLdDocument::toJson(p.person()));
    node.addResult(QJsonArray({res}));
}

