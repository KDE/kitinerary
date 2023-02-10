/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_ELBTICKET_H
#define KITINERARY_ELBTICKET_H

#include "kitinerary_export.h"

#include <QByteArray>
#include <QDateTime>
#include <QMetaType>

#include <optional>

namespace KItinerary {

#define ELB_STR_PROPERTY(Name, Start, Len) \
public: \
    inline QString Name() const { return readString(Start, Len); } \
    Q_PROPERTY(QString Name READ Name)
#define ELB_NUM_PROPERTY(Name, Start, Len) \
public: \
    inline int Name() const { return readNumber(Start, Len); } \
    Q_PROPERTY(int Name READ Name)

class KITINERARY_EXPORT ELBTicketSegment;

/** ERA (Element List Barcode) ELB ticket barcode.
 *  @see ERA TAP TSI TD B.12 Digital Security Elements For Rail Passenger Ticketing - §8 ELB - Element List Barcode
 */
class KITINERARY_EXPORT ELBTicket
{
    Q_GADGET
    // decoding info
    ELB_STR_PROPERTY(idFormat, 0, 1)
    ELB_STR_PROPERTY(pectabCode, 1, 1)
    ELB_STR_PROPERTY(ticketCode, 2, 2)
    ELB_STR_PROPERTY(pnr, 4, 6)
    ELB_STR_PROPERTY(tcnCode, 10, 9)
    ELB_NUM_PROPERTY(specimen, 19, 1)
    ELB_NUM_PROPERTY(barcodeVersion, 20, 1)
    ELB_NUM_PROPERTY(sequenceNumberCurrent, 21, 1)
    ELB_NUM_PROPERTY(sequenceNumberTotal, 22, 1)
    ELB_STR_PROPERTY(futureUse, 23, 10)
    // ticket info
    ELB_STR_PROPERTY(travelerType, 33, 2)
    ELB_NUM_PROPERTY(numberAdults, 35, 2)
    ELB_NUM_PROPERTY(numberChildren, 37, 2)
    ELB_NUM_PROPERTY(year, 39, 1)
    ELB_NUM_PROPERTY(emissionDay, 40, 3)
    ELB_NUM_PROPERTY(beginValidityDay, 43, 3)
    ELB_NUM_PROPERTY(endValidityDay, 46, 3)
    // segments
    Q_PROPERTY(KItinerary::ELBTicketSegment segment1 READ segment1)
    Q_PROPERTY(KItinerary::ELBTicketSegment segment2 READ segment2)

public:
    ~ELBTicket();

    ELBTicketSegment segment1() const;
    ELBTicketSegment segment2() const;

    static bool maybeELBTicket(const QByteArray &data);
    static std::optional<ELBTicket> parse(const QByteArray &data);

    QDate emissionDate(const QDateTime &contextDate = QDateTime::currentDateTime()) const;
    QDate validFromDate(const QDateTime &contextDate = QDateTime::currentDateTime()) const;
    QDate validUntilDate(const QDateTime &contextDate = QDateTime::currentDateTime()) const;

private:
    friend class ELBTicketSegment;
    QString readString(int start, int len) const;
    int readNumber(int start, int len) const;
    QByteArray m_data;
};

/** Segment block of an ERA ELB ticket .*/
class KITINERARY_EXPORT ELBTicketSegment
{
    Q_GADGET
    ELB_STR_PROPERTY(departureStation, 0, 5)
    ELB_STR_PROPERTY(arrivalStation, 5, 5)
    ELB_STR_PROPERTY(trainNumber, 10, 6)
    ELB_STR_PROPERTY(securityCode, 16, 4)
    ELB_NUM_PROPERTY(departureDay, 20, 3)
    ELB_STR_PROPERTY(coachNumber, 23, 3)
    ELB_STR_PROPERTY(seatNumber, 26, 3)
    ELB_STR_PROPERTY(classOfTransport, 29, 1)
    ELB_STR_PROPERTY(tariffCode, 30, 4)
    ELB_STR_PROPERTY(classOfService, 34, 2)
public:
    ~ELBTicketSegment();

    QDate departureDate(const QDateTime &contextDate = QDateTime::currentDateTime()) const;

private:
    friend class ELBTicket;
    QString readString(int start, int len) const;
    int readNumber(int start, int len) const;
    ELBTicket m_ticket;
    int m_offset;
};

#undef ELB_STR_PROPERTY

}

Q_DECLARE_METATYPE(KItinerary::ELBTicket)
Q_DECLARE_METATYPE(KItinerary::ELBTicketSegment)

#endif // KITINERARY_ELBTICKET_H
