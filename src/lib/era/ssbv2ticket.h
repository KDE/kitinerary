/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_SSBV2TICKET_H
#define KITINERARY_SSBV2TICKET_H

#include "kitinerary_export.h"
#include "ssbticketbase.h"

#include <QMetaType>

namespace KItinerary {

/** ERA SSB ticket barcode (version 2).
 *  @see ERA TAP TSI TD B.12 - §15 Appendix B - SSB - old version
 */
class KITINERARY_EXPORT SSBv2Ticket : protected SSBTicketBase
{
    Q_GADGET
    SSB_NUM_PROPERTY(version, 0, 4)
    SSB_NUM_PROPERTY(issuerCode, 4, 14)
    SSB_NUM_PROPERTY(publicKeyVersion, 18, 4)
    SSB_NUM_PROPERTY(rct2TypeIndicator, 22, 1)
    SSB_NUM_PROPERTY(numberOfTickets, 23, 6)
    SSB_NUM_PROPERTY(numberOfAdultPassengers, 29, 7)
    SSB_NUM_PROPERTY(numberOfChildPassengers, 36, 7)
    SSB_NUM_PROPERTY(firstDayOfValidity, 43, 9)
    SSB_NUM_PROPERTY(lastDayOfValidity, 52, 9)
    SSB_NUM_PROPERTY(customerNumberType, 61, 1)
    SSB_LONG_PROPERTY(customerNumber, 62, 47)
    SSB_NUM_PROPERTY(departureStationType, 109, 1)
    SSB_NUM_PROPERTY(departureStationNum, 110, 30)
    SSB_STR_PROPERTY(departureStationAlpha, 110, 5)
    SSB_NUM_PROPERTY(arrivalStationType, 140, 1)
    SSB_NUM_PROPERTY(arrivalStationNum, 141, 30)
    SSB_STR_PROPERTY(arrivalStationAlpha, 141, 5)
    SSB_NUM_PROPERTY(departureTime, 171, 6)
    SSB_NUM_PROPERTY(trainNumber, 177, 25) // alpha 5 encoding??
    SSB_LONG_PROPERTY(reservationReference, 202, 40)
    SSB_NUM_PROPERTY(classOfTransport, 242, 6)
    SSB_NUM_PROPERTY(coachNumber, 248, 10)
    SSB_NUM_PROPERTY(seatNumber, 258, 7) // 250 in trenitalia??
    SSB_STR_PROPERTY(berthNumber, 265, 1)
    SSB_NUM_PROPERTY(overbookingIndicator, 211, 1)
    SSB_STR_PROPERTY(issuerPNRNumber, 272, 7)
    SSB_NUM_PROPERTY(ticketType, 314, 4)
    SSB_NUM_PROPERTY(specimen, 318, 1)
    SSB_STR_PROPERTY(viaStations, 319, 5) // is that the correct encoding? page 131 of TAP TSI Annex B.6 could also be read as 6 times 5 bit content
    SSB_NUM_PROPERTY(railwayCarrierCode, 349, 14)
    SSB_STR_PROPERTY(reference, 363, 8)

    Q_PROPERTY(QByteArray rawData READ rawData STORED false)

public:
    SSBv2Ticket();
    explicit SSBv2Ticket(const QByteArray &data);
    ~SSBv2Ticket();

    /** Returns @c true if this is a valid SSB ticket. */
    bool isValid() const;

    /** Raw barcode data. */
    QByteArray rawData() const;

    /** Returns @c true if @p data might be an ERA SSB ticket. */
    static bool maybeSSB(const QByteArray &data);
};

}

Q_DECLARE_METATYPE(KItinerary::SSBv2Ticket)

#endif // KITINERARY_SSBV2TICKET_H
