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
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KITINERARY_TICKET_H
#define KITINERARY_TICKET_H

#include "kitinerary_export.h"
#include "datatypes.h"

namespace KItinerary {

class SeatPrivate;

/** A reserved seat.
 *  @see https://schema.org/Seat
 */
class KITINERARY_EXPORT Seat
{
    KITINERARY_GADGET(Seat)
    KITINERARY_PROPERTY(QString, seatNumber, setSeatNumber)
    KITINERARY_PROPERTY(QString, seatRow, setSeatRow)
    KITINERARY_PROPERTY(QString, seatSection, setSeatSection)
private:
    detail::shared_data_ptr<SeatPrivate> d;
};

class TicketPrivate;

/** A booked ticket.
 *  @see https://schema.org/Ticket
 */
class KITINERARY_EXPORT Ticket
{
    KITINERARY_GADGET(Ticket)
    KITINERARY_PROPERTY(KItinerary::Seat, ticketedSeat, setTicketedSeat)
    KITINERARY_PROPERTY(QString, ticketToken, setTicketToken)
private:
    detail::shared_data_ptr<TicketPrivate> d;
};

}

Q_DECLARE_METATYPE(KItinerary::Seat)
Q_DECLARE_METATYPE(KItinerary::Ticket)

#endif // KITINERARY_TICKET_H
