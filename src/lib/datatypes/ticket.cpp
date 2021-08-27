/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ticket.h"
#include "datatypes_p.h"

#include <QByteArray>

using namespace KItinerary;

namespace KItinerary {

class SeatPrivate : public QSharedData
{
public:
    QString seatNumber;
    QString seatRow;
    QString seatSection;
    QString seatingType;
};

KITINERARY_MAKE_SIMPLE_CLASS(Seat)
KITINERARY_MAKE_PROPERTY(Seat, QString, seatNumber, setSeatNumber)
KITINERARY_MAKE_PROPERTY(Seat, QString, seatRow, setSeatRow)
KITINERARY_MAKE_PROPERTY(Seat, QString, seatSection, setSeatSection)
KITINERARY_MAKE_PROPERTY(Seat, QString, seatingType, setSeatingType)
KITINERARY_MAKE_OPERATOR(Seat)

class TicketPrivate : public QSharedData
{
public:
    QString name;
    Seat ticketedSeat;
    QString ticketToken;
};

KITINERARY_MAKE_SIMPLE_CLASS(Ticket)
KITINERARY_MAKE_PROPERTY(Ticket, QString, name, setName)
KITINERARY_MAKE_PROPERTY(Ticket, Seat, ticketedSeat, setTicketedSeat)
KITINERARY_MAKE_PROPERTY(Ticket, QString, ticketToken, setTicketToken)
KITINERARY_MAKE_OPERATOR(Ticket)

Ticket::TicketTokenType Ticket::ticketTokenType() const
{
    if (d->ticketToken.startsWith(QLatin1String("qrcode:"), Qt::CaseInsensitive)) {
        return QRCode;
    } else if (d->ticketToken.startsWith(QLatin1String("aztec"), Qt::CaseInsensitive)) {
        return AztecCode;
    } else if (d->ticketToken.startsWith(QLatin1String("barcode128:"), Qt::CaseInsensitive)) {
        return Code128;
    } else if (d->ticketToken.startsWith(QLatin1String("datamatrix:"), Qt::CaseInsensitive)) {
        return DataMatrix;
    } else if (d->ticketToken.startsWith(QLatin1String("pdf417"), Qt::CaseInsensitive)) {
        return PDF417;
    } else if (d->ticketToken.startsWith(QLatin1String("http"), Qt::CaseInsensitive)) {
        return Url;
    }
    return Unknown;
}

QVariant Ticket::ticketTokenData() const
{
    if (d->ticketToken.startsWith(QLatin1String("qrcode:"), Qt::CaseInsensitive)) {
        return ticketToken().mid(7);
    } else if (d->ticketToken.startsWith(QLatin1String("azteccode:"), Qt::CaseInsensitive)) {
        return ticketToken().mid(10);
    } else if (d->ticketToken.startsWith(QLatin1String("aztecbin:"), Qt::CaseInsensitive)) {
        return QByteArray::fromBase64(d->ticketToken.midRef(9).toLatin1());
    } else if (d->ticketToken.startsWith(QLatin1String("barcode128:"), Qt::CaseInsensitive)) {
        return ticketToken().mid(11);
    } else if (d->ticketToken.startsWith(QLatin1String("datamatrix:"), Qt::CaseInsensitive)) {
        return ticketToken().mid(11);
    } else if (d->ticketToken.startsWith(QLatin1String("pdf417:"), Qt::CaseInsensitive)) {
        return ticketToken().mid(7);
    } else if (d->ticketToken.startsWith(QLatin1String("pdf417bin:"), Qt::CaseInsensitive)) {
        return QByteArray::fromBase64(d->ticketToken.midRef(10).toLatin1());
    }
    return ticketToken();
}

}

#include "moc_ticket.cpp"
