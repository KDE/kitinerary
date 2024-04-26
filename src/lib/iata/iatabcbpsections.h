/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kitinerary_export.h"

#include <QDateTime>
#include <QMetaType>

namespace KItinerary {

/** @internal Base class for IATA BCBP sections. */
class KITINERARY_EXPORT IataBcbpSectionBase
{
protected:
    [[nodiscard]] QString readString(int offset, int length) const;
    [[nodiscard]] int readNumericValue(int offset, int length, int base) const;

    QStringView m_data;
};

#define IATA_STR_PROPERTY(Name, Start, Length) \
public: \
    [[nodiscard]] inline QString Name() const { return readString(Start, Length); } \
    Q_PROPERTY(QString Name READ Name)
#define IATA_NUM_PROPERTY(Name, Start, Length) \
public: \
    [[nodiscard]] inline int Name() const { return readNumericValue(Start, Length, 10); } \
    Q_PROPERTY(int Name READ Name)
#define IATA_HEX_PROPERTY(Name, Start, Length) \
public: \
    [[nodiscard]] inline int Name() const { return readNumericValue(Start, Length, 16); } \
    Q_PROPERTY(int Name READ Name)


/** Unique mandatory section of an IATA BCBP. */
class KITINERARY_EXPORT IataBcbpUniqueMandatorySection : protected IataBcbpSectionBase
{
    Q_GADGET
    IATA_STR_PROPERTY(formatCode, 0, 1)
    IATA_NUM_PROPERTY(numberOfLegs, 1, 1)
    IATA_STR_PROPERTY(passengerName, 2, 20)
    IATA_STR_PROPERTY(electronicTicketIndicator, 22, 1)

public:
    IataBcbpUniqueMandatorySection() = default;
    explicit IataBcbpUniqueMandatorySection(QStringView data);
    [[nodiscard]] bool isValid() const;
};

/** Unique conditional (optional) section of an IATA BCBP. */
class KITINERARY_EXPORT IataBcbpUniqueConditionalSection : protected IataBcbpSectionBase
{
    Q_GADGET
    IATA_NUM_PROPERTY(version, 1, 1)
    IATA_HEX_PROPERTY(fieldSize, 2, 2)
    IATA_STR_PROPERTY(passengerDescription, 4, 1)
    IATA_STR_PROPERTY(sourceOfCheckin, 5, 1)
    IATA_STR_PROPERTY(sourceOfBoardingPassIssuance, 6, 1)
    IATA_NUM_PROPERTY(yearOfIssue, 7, 1)
    IATA_NUM_PROPERTY(dayOfIssue, 8, 3)
    IATA_STR_PROPERTY(documentType, 11, 1)
    IATA_STR_PROPERTY(airlineDesignatorOfBoardingPassIssuer, 12, 3)
    IATA_STR_PROPERTY(baggageTagLicensePlateNumber1, 15, 13)
    IATA_STR_PROPERTY(baggageTagLicensePlateNumber2, 28, 13)
    IATA_STR_PROPERTY(baggageTagLicensePlateNumber3, 41, 13)

public:
    IataBcbpUniqueConditionalSection() = default;
    explicit IataBcbpUniqueConditionalSection(QStringView data);
    [[nodiscard]] bool isValid() const;

    Q_INVOKABLE [[nodiscard]] QDate dateOfIssue(const QDateTime &contextDate = QDateTime::currentDateTime()) const;
};

/** Repeated mandatory sections of an IATA BCBP, occurs once per leg. */
class KITINERARY_EXPORT IataBcbpRepeatedMandatorySection : protected IataBcbpSectionBase
{
    Q_GADGET
    IATA_STR_PROPERTY(operatingCarrierPNRCode, 0, 7)
    IATA_STR_PROPERTY(fromCityAirportCode, 7, 3)
    IATA_STR_PROPERTY(toCityAirportCode, 10, 3)
    IATA_STR_PROPERTY(operatingCarrierDesignator, 13, 3)
    IATA_STR_PROPERTY(flightNumber, 16, 5)
    IATA_NUM_PROPERTY(dayOfFlight, 21, 3)
    IATA_STR_PROPERTY(compartmentCode, 24, 1)
    IATA_STR_PROPERTY(seatNumber, 25, 4)
    IATA_STR_PROPERTY(checkinSequenceNumber, 29, 5)
    IATA_STR_PROPERTY(passengerStatus, 34, 1)
    IATA_HEX_PROPERTY(variableFieldSize, 35, 2)

public:
    IataBcbpRepeatedMandatorySection() = default;
    explicit IataBcbpRepeatedMandatorySection(QStringView data);
    [[nodiscard]] bool isValid() const;

    /** Date of the flight.
     *  @param contextDate A date before the flight to determine
     *  the full year which is not specified in the pass itself.
     */
    Q_INVOKABLE [[nodiscard]] QDate dateOfFlight(const QDateTime &contextDate = QDateTime::currentDateTime()) const;
};

/** Conditional (optional) sections of an IATA BCBP, occurs once per leg. */
class KITINERARY_EXPORT IataBcbpRepeatedConditionalSection : protected IataBcbpSectionBase
{
    Q_GADGET
    IATA_HEX_PROPERTY(conditionalFieldSize, 0, 2)
    IATA_STR_PROPERTY(airlineNumericCode, 2, 3)
    IATA_STR_PROPERTY(documentNumber, 5, 10)
    IATA_STR_PROPERTY(selecteeIndicator, 15, 1)
    IATA_STR_PROPERTY(internationalDocumentVerification, 16, 1)
    IATA_STR_PROPERTY(marketingCarrierDesignator, 17, 3)
    IATA_STR_PROPERTY(frequentFlyerAirlineDesignator, 20, 3)
    IATA_STR_PROPERTY(frequenFlyerNumber, 23, 16)
    IATA_STR_PROPERTY(idAdIndicator, 39, 1)
    IATA_STR_PROPERTY(freeBaggageAllowance, 40, 3)
    IATA_STR_PROPERTY(fastTrack, 43, 1)

public:
    IataBcbpRepeatedConditionalSection() = default;
    explicit IataBcbpRepeatedConditionalSection(QStringView data);
};

/** Security section of an IATA BCBP. */
class KITINERARY_EXPORT IataBcbpSecuritySection : protected IataBcbpSectionBase
{
    Q_GADGET
    IATA_STR_PROPERTY(type, 1, 1)
    IATA_HEX_PROPERTY(size, 2, 2)
    IATA_STR_PROPERTY(securityData, 4, size())

public:
    IataBcbpSecuritySection() = default;
    explicit IataBcbpSecuritySection(QStringView data);
};

#undef IATA_STR_PROPERTY
#undef IATA_HEX_PROPERTY

}

