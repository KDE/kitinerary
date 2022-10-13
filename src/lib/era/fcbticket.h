/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_FCBTICKET_H
#define KITINERARY_FCBTICKET_H

#include <KItinerary/Uic9183Block>

#include "asn1/uperelement.h"

#include <QList>
#include <QVariant>

#include <bitset>

namespace KItinerary {

class UPERDecoder;

/** ERA Flexible Content Barcode (FCB)
 *  @see ERA TAP TSI TD B.12 §11 and the corresponding ASN.1 definition
 */
namespace Fcb {

Q_NAMESPACE

class IssuingData {
    UPER_GADGET
    UPER_ELEMENT_OPTIONAL(int, securityProviderNum, 0)
    UPER_ELEMENT_OPTIONAL(QByteArray, securityProviderIA5, 1)
    UPER_ELEMENT_OPTIONAL(int, issuerNum, 2)
    UPER_ELEMENT_OPTIONAL(QByteArray, issuerIA5, 3)
    UPER_ELEMENT(int, issuingYear)
    UPER_ELEMENT(int, issuingDay)
    UPER_ELEMENT_OPTIONAL(int, issuingTime, 4)
    UPER_ELEMENT_OPTIONAL(QString, issuerName, 5)
    UPER_ELEMENT(bool, specimen)
    UPER_ELEMENT(bool, securePaperTicket)
    UPER_ELEMENT(bool, activated)
    UPER_ELEMENT_DEFAULT(QByteArray, currency, "EUR", 6)
    UPER_ELEMENT_DEFAULT(int, currencyFract, 2, 7)
    UPER_ELEMENT_OPTIONAL(QByteArray, issuerPNR, 8)
    // TODO extension ExtensionData OPTIONAL
    UPER_ELEMENT_OPTIONAL(int, issuedOnTrainNum, 10)
    UPER_ELEMENT_OPTIONAL(QByteArray, issuedOnTrainIA5, 11)
    UPER_ELEMENT_OPTIONAL(int, issuedOnLine, 12)
    // TODO pointOfSale GeoCoordinateType OPTIONAL
public:
    void decode(UPERDecoder &decoder);

private:
    std::bitset<14> m_optionals;
};

enum GenderType {
    unspecified = 0,
    female = 1,
    male = 2,
    other = 3,
};
Q_ENUM_NS(GenderType)

enum PassengerType {
    adult = 0,
    senior = 1,
    child = 2,
    youth = 3,
    dog = 4,
    bicycle = 5,
    freeAddonPassenger = 6,
    freeAddonChild = 7,
};
Q_ENUM_NS(PassengerType)

class TravelerType {
    UPER_GADGET
    UPER_ELEMENT_OPTIONAL(QString, firstName, 0)
    UPER_ELEMENT_OPTIONAL(QString, secondName, 1)
    UPER_ELEMENT_OPTIONAL(QString, lastName, 2)
    UPER_ELEMENT_OPTIONAL(QByteArray, idCard, 3)
    UPER_ELEMENT_OPTIONAL(QByteArray, passportId, 4)
    UPER_ELEMENT_OPTIONAL(QByteArray, title, 5)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::GenderType, gender, 6)
    UPER_ELEMENT_OPTIONAL(QByteArray, customerIdIA5, 7)
    UPER_ELEMENT_OPTIONAL(int, customerIdNum, 8)
    UPER_ELEMENT_OPTIONAL(int, yearOfBirth, 9)
    UPER_ELEMENT_OPTIONAL(int, dayOfBirth, 10)
    UPER_ELEMENT(bool, ticketHolder)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::PassengerType, passengerType, 11)
    UPER_ELEMENT_OPTIONAL(bool, passengerWithReducedMobility, 12)
    UPER_ELEMENT_OPTIONAL(int, countryOfResidence, 13)
    UPER_ELEMENT_OPTIONAL(int, countryOfPassport, 14)
    UPER_ELEMENT_OPTIONAL(int, countryOfIdCard, 15)
    // TODO status SEQUENCE OF CustomerStatusType OPTIONAL
public:
    void decode(UPERDecoder &decoder);

private:
    std::bitset<17> m_optionals;
};

class TravelerData {
    UPER_GADGET
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::TravelerType>, traveler, 0)
    UPER_ELEMENT_OPTIONAL(QByteArray, preferredLanguage, 1)
    UPER_ELEMENT_OPTIONAL(QString, groupName, 2)
public:
    void decode(UPERDecoder &decoder);

private:
    std::bitset<3> m_optionals;
};

enum CodeTableType {
    stationUIC = 0,
    stationUICReservation = 1,
    stationERA = 2,
    localCarrierStationCodeTable = 3,
    proprietaryIssuerStationCodeTable = 4,
};
Q_ENUM_NS(CodeTableType)

class TrainLinkType {
    UPER_GADGET
    UPER_ELEMENT_OPTIONAL(int, trainNum, 0)
    UPER_ELEMENT_OPTIONAL(QByteArray, trainIA5, 1)
    UPER_ELEMENT(int, travelDate)
    UPER_ELEMENT(int, departureTime)
    UPER_ELEMENT_OPTIONAL(int, departureUTCOffset, 2)
    UPER_ELEMENT_OPTIONAL(int, fromStationNum, 3)
    UPER_ELEMENT_OPTIONAL(QByteArray, fromStationIA5, 4)
    UPER_ELEMENT_OPTIONAL(int, toStationNum, 5)
    UPER_ELEMENT_OPTIONAL(QByteArray, toStationIA5, 6)
    UPER_ELEMENT_OPTIONAL(QString, fromStationNameUTF8, 7)
    UPER_ELEMENT_OPTIONAL(QString, toStationNameUTF8, 8)
public:
    void decode(UPERDecoder &decoder);
private:
    std::bitset<9> m_optionals;
};

class RegionalValidityType {
    UPER_GADGET
    UPER_ELEMENT(QVariant, value)
public:
    void decode(UPERDecoder &decoder);
};

class ReturnRouteDescriptionType {
    UPER_GADGET
    // TODO
public:
    void decode(UPERDecoder &decoder);
};

enum TravelClassType {
    notApplicable = 0,
    first = 1,
    second = 2,
    tourist = 3,
    comfort = 4,
    premium = 5,
    business = 6,
    all = 7
};
Q_ENUM_NS(TravelClassType)

class OpenTicketData {
    UPER_GADGET
    UPER_ELEMENT_OPTIONAL(int, referenceNum, 0)
    UPER_ELEMENT_OPTIONAL(QByteArray, referenceIA5, 1)
    UPER_ELEMENT_OPTIONAL(int, productOwnerNum, 2)
    UPER_ELEMENT_OPTIONAL(QByteArray, productOwnerIA5, 3)
    UPER_ELEMENT_OPTIONAL(int, productIdNum, 4)
    UPER_ELEMENT_OPTIONAL(QByteArray, productIdIA5, 5)
    UPER_ELEMENT_OPTIONAL(int, extIssuerId, 6)
    UPER_ELEMENT_OPTIONAL(int, issuerAutorizationId, 7)
    UPER_ELEMENT(bool, returnIncluded)
    UPER_ELEMENT_DEFAULT(KItinerary::Fcb::CodeTableType, stationCodeTable, stationUIC, 8)
    UPER_ELEMENT_OPTIONAL(int, fromStationNum, 9)
    UPER_ELEMENT_OPTIONAL(QByteArray, fromStationIA5, 10)
    UPER_ELEMENT_OPTIONAL(int, toStationNum, 11)
    UPER_ELEMENT_OPTIONAL(QByteArray, toStationIA5, 12)
    UPER_ELEMENT_OPTIONAL(QString, fromStationNameUTF8, 13)
    UPER_ELEMENT_OPTIONAL(QString, toStationNameUTF8, 14)
    UPER_ELEMENT_OPTIONAL(QString, validRegionDesc, 15)
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::RegionalValidityType>, validRegion, 16)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::ReturnRouteDescriptionType, returnDescription, 17)
    UPER_ELEMENT_DEFAULT(int, validFromDay, 0, 18)
    UPER_ELEMENT_OPTIONAL(int, validFromTime, 19)
    UPER_ELEMENT_OPTIONAL(int, validFromUTCOffset, 20)
    UPER_ELEMENT_DEFAULT(int, validUntilDay, 0, 21)
    UPER_ELEMENT_OPTIONAL(int, validUntilTime, 22)
    UPER_ELEMENT_OPTIONAL(int, validUntilUTCOffset, 23)
    // TODO activatedDay SEQUENCE OF INTEGER (0..370) OPTIONAL,
    UPER_ELEMENT_DEFAULT(KItinerary::Fcb::TravelClassType, classCode, second, 25)
    UPER_ELEMENT_OPTIONAL(QByteArray, serviceLevel, 26)
    // TODO carrierNum	SEQUENCE OF INTEGER (1..32000)	OPTIONAL
    // TODO carrierIA5	SEQUENCE OF IA5String OPTIONAL
    // TODO includedServiceBrands SEQUENCE OF INTEGER (1..32000) OPTIONAL
    // TODO excludedServiceBrands SEQUENCE OF INTEGER (1..32000) OPTIONAL
    // TODO tariffs SEQUENCE OF TariffType OPTIONAL,
    UPER_ELEMENT_OPTIONAL(int, price, 32)
    // TODO vatDetail SEQUENCE OF VatDetailType OPTIONAL,
    UPER_ELEMENT_OPTIONAL(QString, infoText, 34)
    // TODO includedAddOns SEQUENCE OF IncludedOpenTicketType OPTIONAL
    // TODO luggage LuggageRestrictionType OPTIONAL
    // TODO extension ExtensionData OPTIONAL
public:
    void decode(UPERDecoder &decoder);

private:
    std::bitset<38> m_optionals;
};

class DocumentData {
    UPER_GADGET
    // TODO token  TokenType OPTIONAL
    UPER_ELEMENT(QVariant, ticket)

public:
    void decode(UPERDecoder &decoder);

private:
    std::bitset<1> m_optionals;
};

class KITINERARY_EXPORT UicRailTicketData {
    UPER_GADGET
    UPER_ELEMENT(KItinerary::Fcb::IssuingData, issuingDetail)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::TravelerData, travelerDetail, 0)
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::DocumentData>, transportDocument, 1)
    // TODO
    // controlDetail ControlData OPTIONAL
    // extension SEQUENCE OF ExtensionData OPTIONAL
public:
    UicRailTicketData();
    UicRailTicketData(const Uic9183Block &block);

    void decode(UPERDecoder &decoder);

    static constexpr const char RecordId[] = "U_FLEX";

private:
    Uic9183Block m_block;
    std::bitset<4> m_optionals;
};

}
}

Q_DECLARE_METATYPE(KItinerary::Fcb::IssuingData)
Q_DECLARE_METATYPE(KItinerary::Fcb::TravelerType)
Q_DECLARE_METATYPE(KItinerary::Fcb::TravelerData)
Q_DECLARE_METATYPE(KItinerary::Fcb::RegionalValidityType)
Q_DECLARE_METATYPE(KItinerary::Fcb::ReturnRouteDescriptionType)
Q_DECLARE_METATYPE(KItinerary::Fcb::OpenTicketData)
Q_DECLARE_METATYPE(KItinerary::Fcb::DocumentData)
Q_DECLARE_METATYPE(KItinerary::Fcb::UicRailTicketData)

#endif // KITINERARY_FCBTICKET_H
