/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ticket.h"
#include "datatypes_p.h"

#include <QByteArray>

#include <cmath>

using namespace KItinerary;

namespace KItinerary {

class SeatPrivate : public QSharedData
{
public:
    QString seatNumber;
    QString seatRow;
    QString seatSection;
    QString seatingType;
    QString identifier;
};

KITINERARY_MAKE_CLASS(Seat)
KITINERARY_MAKE_PROPERTY(Seat, QString, seatNumber, setSeatNumber)
KITINERARY_MAKE_PROPERTY(Seat, QString, seatRow, setSeatRow)
KITINERARY_MAKE_PROPERTY(Seat, QString, seatSection, setSeatSection)
KITINERARY_MAKE_PROPERTY(Seat, QString, seatingType, setSeatingType)
KITINERARY_MAKE_PROPERTY(Seat, QString, identifier, setIdentifier)
KITINERARY_MAKE_OPERATOR(Seat)

class TicketPrivate : public QSharedData
{
public:
    QString name;
    Organization issuedBy;
    QString ticketNumber;
    Seat ticketedSeat;
    QString ticketToken;
    Person underName;
    QVariantList subjectOf;
    QDateTime validFrom;
    QDateTime validUntil;
    double totalPrice = NAN;
    QString priceCurrency;
    QString identifier;
};

KITINERARY_MAKE_CLASS(Ticket)
KITINERARY_MAKE_PROPERTY(Ticket, QString, name, setName)
KITINERARY_MAKE_PROPERTY(Ticket, Organization, issuedBy, setIssuedBy)
KITINERARY_MAKE_PROPERTY(Ticket, QString, ticketNumber, setTicketNumber)
KITINERARY_MAKE_PROPERTY(Ticket, Seat, ticketedSeat, setTicketedSeat)
KITINERARY_MAKE_PROPERTY(Ticket, QString, ticketToken, setTicketToken)
KITINERARY_MAKE_PROPERTY(Ticket, Person, underName, setUnderName)
KITINERARY_MAKE_PROPERTY(Ticket, QVariantList, subjectOf, setSubjectOf)
KITINERARY_MAKE_PROPERTY(Ticket, QDateTime, validFrom, setValidFrom)
KITINERARY_MAKE_PROPERTY(Ticket, QDateTime, validUntil, setValidUntil)
KITINERARY_MAKE_PROPERTY(Ticket, double, totalPrice, setTotalPrice)
KITINERARY_MAKE_PROPERTY(Ticket, QString, priceCurrency, setPriceCurrency)
KITINERARY_MAKE_PROPERTY(Ticket, QString, identifier, setIdentifier)
KITINERARY_MAKE_OPERATOR(Ticket)

Token::TokenType Ticket::ticketTokenType() const
{
    return Token::tokenType(d->ticketToken);
}

QVariant Ticket::ticketTokenData() const
{
    return Token::tokenData(d->ticketToken);
}

}

#include "moc_ticket.cpp"
