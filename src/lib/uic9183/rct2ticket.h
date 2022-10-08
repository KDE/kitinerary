/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kitinerary_export.h"

#include <QExplicitlySharedDataPointer>
#include <QMetaType>

class QDateTime;

namespace KItinerary {

class Rct2TicketPrivate;
class Uic9183TicketLayout;

/** RCT2 ticket layout payload of an UIC 918.3 ticket token. */
class KITINERARY_EXPORT Rct2Ticket
{
    Q_GADGET
    Q_PROPERTY(QDate firstDayOfValidity READ firstDayOfValidity)
    Q_PROPERTY(Type type READ type)
    Q_PROPERTY(QString title READ title)
    Q_PROPERTY(QString passengerName READ passengerName)

    Q_PROPERTY(QDateTime outboundDepartureTime READ outboundDepartureTime)
    Q_PROPERTY(QDateTime outboundArrivalTime READ outboundArrivalTime)
    Q_PROPERTY(QString outboundDepartureStation READ outboundDepartureStation)
    Q_PROPERTY(QString outboundArrivalStation READ outboundArrivalStation)
    Q_PROPERTY(QString outboundClass READ outboundClass)

    Q_PROPERTY(QDateTime returnDepartureTime READ returnDepartureTime)
    Q_PROPERTY(QDateTime returnArrivalTime READ returnArrivalTime)
    Q_PROPERTY(QString returnDepartureStation READ returnDepartureStation)
    Q_PROPERTY(QString returnArrivalStation READ returnArrivalStation)
    Q_PROPERTY(QString returnClass READ returnClass)

    Q_PROPERTY(QString trainNumber READ trainNumber)
    Q_PROPERTY(QString coachNumber READ coachNumber)
    Q_PROPERTY(QString seatNumber READ seatNumber)

public:
    Rct2Ticket();
    /** Parse RCT2 ticket information from a U_TLAY layout block. */
    Rct2Ticket(const Uic9183TicketLayout &layout);
    Rct2Ticket(const Rct2Ticket&);
    ~Rct2Ticket();
    Rct2Ticket& operator=(const Rct2Ticket&);

    /** Returns whether this is a valid RCT2 ticket layout block. */
    bool isValid() const;

    /** Date/time this ticket was first encountered, to recover possibly missing year numbers. */
    void setContextDate(const QDateTime &contextDt);

    /** First day the ticket is valid. */
    QDate firstDayOfValidity() const;

    /** Type of RCT2 ticket.
     *  @see ERA TAP TSI Annex B.6.
     */
    enum Type {
        Transport, ///< Non-integrated Reservation Ticket (NRT)
        TransportReservation, ///< Integration Reservation Ticket (IRT)
        Reservation, ///< Reservation Only Document (RES)
        Upgrade, ///< Update Document (UPG)
        RailPass, ///< Rail Pass Ticket (RPT)
        Unknown ///< ticket type could not be detected, or ticket type not supported yet
    };
    Q_ENUM(Type)
    /** Returns the ticket type. */
    Type type() const;

    /** Name or title of the ticket. */
    QString title() const;

    /** Name of the passenger this ticket is for. */
    QString passengerName() const;

    /** Departure time of the outbound segment. */
    QDateTime outboundDepartureTime() const;
    /** Arrival time of the outbound segment. */
    QDateTime outboundArrivalTime() const;
    /** Departure station of the outbound segment. */
    QString outboundDepartureStation() const;
    /** Arrival station of the outbound segment. */
    QString outboundArrivalStation() const;
    /** Class of the outbound segment. */
    QString outboundClass() const;

    /** Departure time of the return segment, if present. */
    QDateTime returnDepartureTime() const;
    /** Arrival time of the return segment, if present. */
    QDateTime returnArrivalTime() const;
    /** Departure station of the return segment, if present. */
    QString returnDepartureStation() const;
    /** Arrival station of the return segment, if present. */
    QString returnArrivalStation() const;
    /** Class of the return segment, if present. */
    QString returnClass() const;

    /** Train number (for reservation tickets). */
    QString trainNumber() const;
    /** Coach number (for reservation tickets). */
    QString coachNumber() const;
    /** Seat number (for reservation tickets). */
    QString seatNumber() const;

private:
    QExplicitlySharedDataPointer<Rct2TicketPrivate> d;
};

}

Q_DECLARE_METATYPE(KItinerary::Rct2Ticket)

