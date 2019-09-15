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
    KITINERARY_PROPERTY(QString, seatingType, setSeatingType)
private:
    QExplicitlySharedDataPointer<SeatPrivate> d;
};

class TicketPrivate;

/** A booked ticket.
 *  @see https://schema.org/Ticket
 */
class KITINERARY_EXPORT Ticket
{
    KITINERARY_GADGET(Ticket)
    KITINERARY_PROPERTY(KItinerary::Seat, ticketedSeat, setTicketedSeat)
    /** The raw ticket token string.
     *  @see ticketTokenType, ticketTokenData
     */
    KITINERARY_PROPERTY(QString, ticketToken, setTicketToken)

    /** The type of the content in ticketToken. */
    Q_PROPERTY(TicketTokenType ticketTokenType READ ticketTokenType STORED false)
    /** The ticket token payload for barcodes, otherwise the same as ticketToken. */
    Q_PROPERTY(QString ticketTokenData READ ticketTokenData STORED false)

public:
    /** The type of content in the ticketToken property. */
    enum TicketTokenType {
        Unknown, ///< Unknown or empty ticket token
        Url, ///< A download URL
        QRCode, ///< QR code
        AztecCode, ///< Aztec code
        Code128, ///< Code 128 barcode
        DataMatrix ///< A DataMatrix barcode
    };
    Q_ENUM(TicketTokenType)

    TicketTokenType ticketTokenType() const;
    QString ticketTokenData() const;
private:
    QExplicitlySharedDataPointer<TicketPrivate> d;
};

}

Q_DECLARE_METATYPE(KItinerary::Seat)
Q_DECLARE_METATYPE(KItinerary::Ticket)

#endif // KITINERARY_TICKET_H
