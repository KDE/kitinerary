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

#ifndef KITINERARY_UIC9183PARSER_H
#define KITINERARY_UIC9183PARSER_H

#include "kitinerary_export.h"

#include <KItinerary/Person>

#include <QExplicitlySharedDataPointer>
#include <QMetaType>

class QDateTime;

namespace KItinerary {

class Rct2TicketPrivate;
class Uic9183Block;
class Uic9183Parser;

/** RCT2 ticket layout payload of an UIC 918.3 ticket token. */
class KITINERARY_EXPORT Rct2Ticket
{
    Q_GADGET
    Q_PROPERTY(QDate firstDayOfValidity READ firstDayOfValidity)
    Q_PROPERTY(Type type READ type)
    Q_PROPERTY(QDateTime outboundDepartureTime READ outboundDepartureTime)
    Q_PROPERTY(QDateTime outboundArrivalTime READ outboundArrivalTime)
    Q_PROPERTY(QString outboundDepartureStation READ outboundDepartureStation)
    Q_PROPERTY(QString outboundArrivalStation READ outboundArrivalStation)
    Q_PROPERTY(QString outboundClass READ outboundClass)
    Q_PROPERTY(QString trainNumber READ trainNumber)
    Q_PROPERTY(QString coachNumber READ coachNumber)
    Q_PROPERTY(QString seatNumber READ seatNumber)

public:
    Rct2Ticket();
    Rct2Ticket(const Rct2Ticket&);
    ~Rct2Ticket();
    Rct2Ticket& operator=(const Rct2Ticket&);

    /** Returns whether this is a valid RCT2 ticket layout block. */
    bool isValid() const;

    /** Date/time this ticket was first encounted, to recover possibly missing year numbers. */
    void setContextDate(const QDateTime &contextDt);

    /** First day the ticket is valid. */
    QDate firstDayOfValidity() const;

    /** Type of RCT2 ticket.
     *  @see ERA TAP TSI Annex B.6.
     */
    enum Type {
        Reservation, ///< an reservation-only ticket (RES)
        Unknown ///< ticket type could not be detected, or ticket type not supported yet
    };
    Q_ENUM(Type);
    /** Returns the ticket type. */
    Type type() const;

    /** Departure time of the outbound segment. */
    QDateTime outboundDepartureTime() const;
    /** Arrival time of the outbound segment. */
    QDateTime outboundArrivalTime() const;
    /** Departure station of the outbound segment. */
    QString outboundDepartureStation() const;
    /** Arrival station of the outbound segement. */
    QString outboundArrivalStation() const;
    /** Class of the outbound segment. */
    QString outboundClass() const;

    /** Train number (for reservation tickets). */
    QString trainNumber() const;
    /** Coach number (for reservation tickets). */
    QString coachNumber() const;
    /** Seat number (for reservation tickets). */
    QString seatNumber() const;

private:
    friend class Uic9183Parser;
    Rct2Ticket(Uic9183Block block);
    QExplicitlySharedDataPointer<Rct2TicketPrivate> d;
};

class Uic9183ParserPrivate;

/** Parser for UIC 918.3 and 918.3* train tickets.
 *
 *  @see http://www.era.europa.eu/Document-Register/Documents/ERA_Technical_Document_TAP_B_7_v1.2.pdf
 *    for information about the general UIC 918-3 structure
 *  @see http://www.era.europa.eu/Document-Register/Documents/ERA_Technical_Document_TAP_B_6_v1_2.pdf
 *    for information about the U_TLAY block
 *  @see https://www.bahn.de/p/view/angebot/regio/barcode.shtml
 *    for information about the 0080VU vendor block
 */
class KITINERARY_EXPORT Uic9183Parser
{
    Q_GADGET
    Q_PROPERTY(QString pnr READ pnr)
    Q_PROPERTY(QString carrierId READ carrierId)
    Q_PROPERTY(KItinerary::Person person READ person)
    Q_PROPERTY(QString outboundDepartureStationId READ outboundDepartureStationId)
    Q_PROPERTY(QString outboundArrivalStationId READ outboundArrivalStationId)
    /** RCT2 ticket layout block, if present, @c null otherwise. */
    Q_PROPERTY(QVariant rct2Ticket READ rct2TicketVariant)

public:
    Uic9183Parser();
    Uic9183Parser(const Uic9183Parser&);
    ~Uic9183Parser();
    Uic9183Parser& operator=(const Uic9183Parser&);

    /** Date/time this ticket was first encountered.
     *  This is used to recover a missing year in the ticket data.
     */
    void setContextDate(const QDateTime &contextDt);

    void parse(const QByteArray &data);
    bool isValid() const;

    /** The booking reference. */
    QString pnr() const;
    /** The UIC carrier code. */
    QString carrierId() const;
    /** The person this ticket is issued to. */
    Person person() const;

    /** Station identifier for the departure station of the outbound trip. */
    QString outboundDepartureStationId() const;
    /** Station identifier for the arrival station of the outbound trip. */
    QString outboundArrivalStationId() const;

    /** RCT2 ticket layout, if present. */
    Rct2Ticket rct2Ticket() const;

    /** Quickly checks if @p might be UIC 918.3 content.
     *  This priorizes speed over correctness and is used in barcode content auto-detection.
     */
    static bool maybeUic9183(const QByteArray &data);

private:
    QVariant rct2TicketVariant() const;
    QExplicitlySharedDataPointer<Uic9183ParserPrivate> d;
};

}

Q_DECLARE_METATYPE(KItinerary::Rct2Ticket)
Q_DECLARE_METATYPE(KItinerary::Uic9183Parser)

#endif // KITINERARY_UIC9183PARSER_H
