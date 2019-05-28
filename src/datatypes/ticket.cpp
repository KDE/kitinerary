/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
    Seat ticketedSeat;
    QString ticketToken;
};

KITINERARY_MAKE_SIMPLE_CLASS(Ticket)
KITINERARY_MAKE_PROPERTY(Ticket, Seat, ticketedSeat, setTicketedSeat)
KITINERARY_MAKE_PROPERTY(Ticket, QString, ticketToken, setTicketToken)
KITINERARY_MAKE_OPERATOR(Ticket)

Ticket::TicketTokenType Ticket::ticketTokenType() const
{
    if (d->ticketToken.startsWith(QLatin1Literal("qrcode:"), Qt::CaseInsensitive)) {
        return QRCode;
    } else if (d->ticketToken.startsWith(QLatin1String("aztec"), Qt::CaseInsensitive)) {
        return AztecCode;
    } else if (d->ticketToken.startsWith(QLatin1String("barcode128:"), Qt::CaseInsensitive)) {
        return Code128;
    } else if (d->ticketToken.startsWith(QLatin1String("http"), Qt::CaseInsensitive)) {
        return Url;
    }
    return Unknown;
}

QString Ticket::ticketTokenData() const
{
    if (d->ticketToken.startsWith(QLatin1Literal("qrcode:"), Qt::CaseInsensitive)) {
        return ticketToken().mid(7);
    } else if (d->ticketToken.startsWith(QLatin1String("azteccode:"), Qt::CaseInsensitive)) {
        return ticketToken().mid(10);
    } else if (d->ticketToken.startsWith(QLatin1String("aztecbin:"), Qt::CaseInsensitive)) {
        const auto b = QByteArray::fromBase64(d->ticketToken.midRef(9).toLatin1());
        return QString::fromLatin1(b.constData(), b.size());
    } else if (d->ticketToken.startsWith(QLatin1String("barcode128:"), Qt::CaseInsensitive)) {
        return ticketToken().mid(11);
    }
    return ticketToken();
}

}

#include "moc_ticket.cpp"
