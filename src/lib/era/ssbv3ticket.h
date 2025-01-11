/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kitinerary_export.h"
#include "ssbticketbase.h"

#include <QDateTime>
#include <QMetaType>

namespace KItinerary {

/** ERA SSB ticket barcode (version 3).
 *  @see ERA TAP TSI TD B.12 Digital Security Elements For Rail Passenger Ticketing - §7 SSB - Small Structured Barcode
 */
class KITINERARY_EXPORT SSBv3Ticket : protected SSBTicketBase {
    Q_GADGET
    // low-level raw value access
    // header
    SSB_NUM_PROPERTY(version, 0, 4)
    SSB_NUM_PROPERTY(issuerCode, 4, 14)
    SSB_NUM_PROPERTY(id, 18, 4)
    SSB_NUM_PROPERTY(ticketTypeCode, 22, 5)

    // open data common part for type 1-4
    SSB_NUM_PROPERTY(numberOfAdultPassengers, 27, 7)
    SSB_NUM_PROPERTY(numberOfChildPassengers, 34, 7)
    SSB_NUM_PROPERTY(specimen, 41, 1)
    SSB_NUM_PROPERTY(classOfTravel, 42, 6)
    SSB_STR_PROPERTY(tcn, 48, 14)
    SSB_NUM_PROPERTY(yearOfIssue, 132, 4) // last digit only
    SSB_NUM_PROPERTY(issuingDay, 136, 9)

    // open data type 1 (IRT, RES, BOA) variant
    SSB_NUM_PROPERTY(type1SubTicketType, 145, 2)
    SSB_NUM_PROPERTY(type1StationCodeNumericOrAlpha, 147, 1)
    SSB_NUM_PROPERTY(type1StationCodeListType, 148, 4)
    SSB_NUM_PROPERTY(type1DepartureStationNum, 152, 28)
    SSB_STR_PROPERTY(type1DepartureStationAlpha, 148, 5)
    SSB_NUM_PROPERTY(type1ArrivalStationNum, 180, 28)
    SSB_STR_PROPERTY(type1ArrivalStationAlpha, 178, 5)
    SSB_NUM_PROPERTY(type1DepartureDate, 208, 9)
    SSB_NUM_PROPERTY(type1DepartureTime, 217, 11)
    SSB_STR_PROPERTY(type1TrainNumber, 228, 5)
    SSB_NUM_PROPERTY(type1CoachNumber, 258, 10)
    SSB_STR_PROPERTY(type1SeatNumber, 268, 3)
    SSB_NUM_PROPERTY(type1OverbookingIndicator, 286, 1)
    SSB_NUM_PROPERTY(type1InformationMessages, 287, 14)
    SSB_STR_PROPERTY(type1OpenText, 301, 27)

    // open data type 2 (NRT) variant
    SSB_NUM_PROPERTY(type2ReturnJourneyFlag, 145, 1)
    SSB_NUM_PROPERTY(type2FirstDayOfValidity, 146, 9)
    SSB_NUM_PROPERTY(type2LastDayOfValidity, 155, 9)
    SSB_NUM_PROPERTY(type2StationCodeNumericOrAlpha, 164, 1)
    SSB_NUM_PROPERTY(type2StationCodeListType, 165, 4)
    SSB_NUM_PROPERTY(type2DepartureStationNum, 169, 28)
    SSB_STR_PROPERTY(type2DepartureStationAlpha, 165, 5)
    SSB_NUM_PROPERTY(type2ArrivalStationNum, 197, 28)
    SSB_STR_PROPERTY(type2ArrivalStationAlpha, 195, 5)
    SSB_NUM_PROPERTY(type2InformationMessages, 225, 14)
    SSB_STR_PROPERTY(type2OpenText, 239, 37)

    // open data type 3 (GRT) variant
    SSB_NUM_PROPERTY(type3ReturnJourneyFlag, 145, 1)
    SSB_NUM_PROPERTY(type3FirstDayOfValidity, 146, 9)
    SSB_NUM_PROPERTY(type3LastDayOfValidity, 155, 9)
    SSB_NUM_PROPERTY(type3StationCodeNumericOrAlpha, 164, 1)
    SSB_NUM_PROPERTY(type3StationCodeListType, 165, 4)
    SSB_NUM_PROPERTY(type3DepartureStationNum, 169, 28)
    SSB_STR_PROPERTY(type3DepartureStationAlpha, 165, 5)
    SSB_NUM_PROPERTY(type3ArrivalStationNum, 197, 28)
    SSB_STR_PROPERTY(type3ArrivalStationAlpha, 195, 5)
    SSB_STR_PROPERTY(type3NameOfGroupLeader, 225, 12)
    SSB_NUM_PROPERTY(type3CountermarkNumber, 297, 8)
    SSB_NUM_PROPERTY(type3InformationMessages, 305, 14)
    SSB_STR_PROPERTY(type3OpenText, 305, 24)

    // open data type 4 (RPT) variant
    SSB_NUM_PROPERTY(type4RPTSubTicketType, 145, 2)
    SSB_NUM_PROPERTY(type4FirstDayOfValidity, 147, 9)
    SSB_NUM_PROPERTY(type4LastDayOfValidity, 156, 9)
    SSB_NUM_PROPERTY(type4NumberOfDaysOfTravelAllowed, 165, 7)
    SSB_NUM_PROPERTY(type4CountryCode1, 172, 7)
    SSB_NUM_PROPERTY(type4CountryCode2, 179, 7)
    SSB_NUM_PROPERTY(type4CountryCode3, 186, 7)
    SSB_NUM_PROPERTY(type4CountryCode4, 193, 7)
    SSB_NUM_PROPERTY(type4SecondPage, 200, 1)
    SSB_NUM_PROPERTY(type4InformationMessages, 201, 14)
    SSB_STR_PROPERTY(type4OpenText, 215, 40)

    enum TicketType {
        IRT_RES_BOA = 1,
        NRT = 2,
        GRT = 3,
        RPT = 4
    };
    Q_ENUM(TicketType)

    Q_PROPERTY(QByteArray rawData READ rawData STORED false)

public:
    SSBv3Ticket();
    explicit SSBv3Ticket(const QByteArray &data);
    ~SSBv3Ticket();

    /** Returns @c true if this is a valid SSB ticket. */
    [[nodiscard]] bool isValid() const;

    /** Date of issue. */
    Q_INVOKABLE [[nodiscard]] QDate issueDate(const QDateTime &contextDate = QDateTime::currentDateTime()) const;
    /** Departure day for type 1 (IRT/RES/BOA) tickets. */
    Q_INVOKABLE [[nodiscard]] QDate type1DepartureDay(const QDateTime &contextDate = QDateTime::currentDateTime()) const;
    /** First day of validity for type 2 (NRT) tickets. */
    Q_INVOKABLE [[nodiscard]] QDate type2ValidFrom(const QDateTime &contextDate = QDateTime::currentDateTime()) const;
    /** Last day of validity for type 2 (NRT) tickets. */
    Q_INVOKABLE [[nodiscard]] QDate type2ValidUntil(const QDateTime &contextDate = QDateTime::currentDateTime()) const;
    /** First day of validity for type 3 (GRP) tickets. */
    Q_INVOKABLE [[nodiscard]] QDate type3ValidFrom(const QDateTime &contextDate = QDateTime::currentDateTime()) const;
    /** Last day of validity for type 3 (GRP) tickets. */
    Q_INVOKABLE [[nodiscard]] QDate type3ValidUntil(const QDateTime &contextDate = QDateTime::currentDateTime()) const;

    /** Raw barcode data. */
    [[nodiscard]] QByteArray rawData() const;

    /** Returns @c true if @p data might be an ERA SSB ticket. */
    [[nodiscard]] static bool maybeSSB(const QByteArray &data);

private:
    [[nodiscard]] QString readString(int start, int length) const;
};

}

Q_DECLARE_METATYPE(KItinerary::SSBv3Ticket)

