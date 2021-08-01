/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kitinerary_export.h"
#include "ssbticketbase.h"

#include <QDate>
#include <QMetaType>

namespace KItinerary {

/** ERA SSB ticket barcode (version 1).
 *  @see ERA TAP TSI Annex B.6 - Appendix C.1
 */
class KITINERARY_EXPORT SSBv1Ticket : protected SSBTicketBase
{
    Q_GADGET
    SSB_NUM_PROPERTY(version, 0, 4)
    SSB_NUM_PROPERTY(issuerCode, 4, 14)
    SSB_NUM_PROPERTY(rct2TypeIndicator, 18, 1)
    SSB_NUM_PROPERTY(numberOfTickets, 19, 6)
    SSB_NUM_PROPERTY(numberOfAdultPassengers, 25, 7)
    SSB_NUM_PROPERTY(numberOfChildPassengers, 32, 7)
    /** Days since Jan 01. */
    SSB_NUM_PROPERTY(firstDayOfValidityDay, 39, 9)
    SSB_NUM_PROPERTY(lastDayOfValidityDay, 48, 9)
    SSB_NUM_PROPERTY(customerNumberType, 57, 1)
    SSB_LONG_PROPERTY(customerNumber, 58, 47)
    SSB_NUM_PROPERTY(departureStationType, 105, 1)
    SSB_NUM_PROPERTY(departureStationNum, 106, 30)
    SSB_STR_PROPERTY(departureStationAlpha, 106, 5)
    SSB_NUM_PROPERTY(arrivalStationType, 136, 1)
    SSB_NUM_PROPERTY(arrivalStationNum, 137, 30)
    SSB_STR_PROPERTY(arrivalStationAlpha, 137, 5)
    /** departure time encoded as 48 30min time slots */
    SSB_NUM_PROPERTY(departureTimeSlot, 167, 6)
    SSB_NUM_PROPERTY(trainNumber, 173, 17)
    SSB_LONG_PROPERTY(reservationReference, 190, 40)
    /** ERA/TD/2009-14/INT: PASSENGER CODE LIST TO TAP TSI - §B.5.24 - Class
     *  Possible values are: 1 (1st), 2 (2nd), C (Club), P (Superior), T (Tourist)
     */
    SSB_NUM_PROPERTY(classOfTransport, 230, 6)
    SSB_NUM_PROPERTY(coachNumber, 236, 10)
    SSB_NUM_PROPERTY(seatNumber, 246, 7)
    SSB_STR_PROPERTY(berthNumber, 253, 1)
    SSB_NUM_PROPERTY(overbookingIndicator, 259, 1)
    SSB_STR_PROPERTY(issuerPNRNumber, 260, 7)
    SSB_NUM_PROPERTY(ticketType, 302, 4)
    SSB_NUM_PROPERTY(specimen, 306, 1)
    SSB_STR_PROPERTY(viaStations, 307, 5) // is that the correct encoding? page 131 of TAP TSI Annex B.6 could also be read as 6 times 5 bit content

    Q_PROPERTY(QByteArray rawData READ rawData STORED false)

public:
    SSBv1Ticket();
    explicit SSBv1Ticket(const QByteArray &data);
    ~SSBv1Ticket();

    /** Returns @c true if this is a valid SSB ticket. */
    bool isValid() const;

    /** First day of validity. */
    Q_INVOKABLE QDate firstDayOfValidity(const QDate &contextDate = QDate::currentDate()) const;
    /** Decoded departure time slot. */
    Q_INVOKABLE QDateTime departureTime(const QDate &contextDate = QDate::currentDate()) const;

    /** Raw barcode data. */
    QByteArray rawData() const;

    /** Returns @c true if @p data might be an ERA SSB ticket. */
    static bool maybeSSB(const QByteArray &data);
};

}

Q_DECLARE_METATYPE(KItinerary::SSBv1Ticket)

