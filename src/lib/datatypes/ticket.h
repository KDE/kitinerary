/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kitinerary_export.h"
#include "datatypes.h"
#include "organization.h"
#include "person.h"
#include "token.h"

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

    KITINERARY_PROPERTY(QString, identifier, setIdentifier)
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
    KITINERARY_PROPERTY(QString, name, setName)
    KITINERARY_PROPERTY(KItinerary::Organization, issuedBy, setIssuedBy)
    KITINERARY_PROPERTY(QString, ticketNumber, setTicketNumber)
    KITINERARY_PROPERTY(KItinerary::Seat, ticketedSeat, setTicketedSeat)
    /** The raw ticket token string.
     *  @see ticketTokenType, ticketTokenData
     */
    KITINERARY_PROPERTY(QString, ticketToken, setTicketToken)

    /** The type of the content in ticketToken. */
    Q_PROPERTY(KItinerary::Token::TokenType ticketTokenType READ ticketTokenType STORED false)
    /** The ticket token payload for barcodes, otherwise the same as ticketToken.
     *  For binary content barcodes this is a QByteArray, otherwise a QString.
     */
    Q_PROPERTY(QVariant ticketTokenData READ ticketTokenData STORED false)

    KITINERARY_PROPERTY(KItinerary::Person, underName, setUnderName)
    KITINERARY_PROPERTY(QVariantList, subjectOf, setSubjectOf)

    /** Non-standard extension for ticket validity time ranges. */
    KITINERARY_PROPERTY(QDateTime, validFrom, setValidFrom)
    KITINERARY_PROPERTY(QDateTime, validUntil, setValidUntil)

    KITINERARY_PROPERTY(double, totalPrice, setTotalPrice)
    KITINERARY_PROPERTY(QString, priceCurrency, setPriceCurrency)

    KITINERARY_PROPERTY(QString, identifier, setIdentifier)

public:
    /** The type of content in the ticketToken property.
     *  @deprecated Use Token::TokenType instead.
     */
    enum TicketTokenType {
        Unknown = Token::Unknown, ///< Unknown or empty ticket token
        Url = Token::Url, ///< A download URL
        QRCode = Token::QRCode, ///< QR code
        AztecCode = Token::AztecCode, ///< Aztec code
        Code128 = Token::Code128, ///< Code 128 barcode
        DataMatrix = Token::DataMatrix, ///< A DataMatrix barcode
        PDF417 = Token::PDF417, ///< A PDF417 barcode
    };
    Q_ENUM(TicketTokenType)

    Token::TokenType ticketTokenType() const;
    QVariant ticketTokenData() const;
private:
    QExplicitlySharedDataPointer<TicketPrivate> d;
};

}

Q_DECLARE_METATYPE(KItinerary::Seat)
Q_DECLARE_METATYPE(KItinerary::Ticket)

