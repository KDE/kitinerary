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

class ExtensionData {
    UPER_GADGET
    UPER_ELEMENT(QByteArray, extensionId)
    UPER_ELEMENT(QByteArray, extensionData)
    UPER_GADGET_FINALIZE
};

class GeoCoordinateType {
    UPER_GADGET
    // TODO
};

class IssuingData {
    UPER_EXTENDABLE_GADGET
    UPER_ELEMENT_OPTIONAL(int, securityProviderNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, securityProviderIA5)
    UPER_ELEMENT_OPTIONAL(int, issuerNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, issuerIA5)
    UPER_ELEMENT(int, issuingYear)
    UPER_ELEMENT(int, issuingDay)
    UPER_ELEMENT_OPTIONAL(int, issuingTime)
    UPER_ELEMENT_OPTIONAL(QString, issuerName)
    UPER_ELEMENT(bool, specimen)
    UPER_ELEMENT(bool, securePaperTicket)
    UPER_ELEMENT(bool, activated)
    UPER_ELEMENT_DEFAULT(QByteArray, currency, "EUR")
    UPER_ELEMENT_DEFAULT(int, currencyFract, 2)
    UPER_ELEMENT_OPTIONAL(QByteArray, issuerPNR)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::ExtensionData, extension)
    UPER_ELEMENT_OPTIONAL(int, issuedOnTrainNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, issuedOnTrainIA5)
    UPER_ELEMENT_OPTIONAL(int, issuedOnLine)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::GeoCoordinateType, pointOfSale)
    UPER_GADGET_FINALIZE
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

class CustomerStatusType {
    UPER_GADGET
    UPER_ELEMENT_OPTIONAL(int, statusProviderNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, statusProviderIA5)
    UPER_ELEMENT_OPTIONAL(int, customStatus)
    UPER_ELEMENT_OPTIONAL(QByteArray, customerStatusDescr)
    UPER_GADGET_FINALIZE
};

class TravelerType {
    UPER_EXTENDABLE_GADGET
    UPER_ELEMENT_OPTIONAL(QString, firstName)
    UPER_ELEMENT_OPTIONAL(QString, secondName)
    UPER_ELEMENT_OPTIONAL(QString, lastName)
    UPER_ELEMENT_OPTIONAL(QByteArray, idCard)
    UPER_ELEMENT_OPTIONAL(QByteArray, passportId)
    UPER_ELEMENT_OPTIONAL(QByteArray, title)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::GenderType, gender)
    UPER_ELEMENT_OPTIONAL(QByteArray, customerIdIA5)
    UPER_ELEMENT_OPTIONAL(int, customerIdNum)
    UPER_ELEMENT_OPTIONAL(int, yearOfBirth)
    UPER_ELEMENT_OPTIONAL(int, dayOfBirth)
    UPER_ELEMENT(bool, ticketHolder)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::PassengerType, passengerType)
    UPER_ELEMENT_OPTIONAL(bool, passengerWithReducedMobility)
    UPER_ELEMENT_OPTIONAL(int, countryOfResidence)
    UPER_ELEMENT_OPTIONAL(int, countryOfPassport)
    UPER_ELEMENT_OPTIONAL(int, countryOfIdCard)
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::CustomerStatusType>, status)
    UPER_GADGET_FINALIZE
};

class TravelerData {
    UPER_EXTENDABLE_GADGET
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::TravelerType>, traveler)
    UPER_ELEMENT_OPTIONAL(QByteArray, preferredLanguage)
    UPER_ELEMENT_OPTIONAL(QString, groupName)
    UPER_GADGET_FINALIZE
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
    UPER_ELEMENT_OPTIONAL(int, trainNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, trainIA5)
    UPER_ELEMENT(int, travelDate)
    UPER_ELEMENT(int, departureTime)
    UPER_ELEMENT_OPTIONAL(int, departureUTCOffset)
    UPER_ELEMENT_OPTIONAL(int, fromStationNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, fromStationIA5)
    UPER_ELEMENT_OPTIONAL(int, toStationNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, toStationIA5)
    UPER_ELEMENT_OPTIONAL(QString, fromStationNameUTF8)
    UPER_ELEMENT_OPTIONAL(QString, toStationNameUTF8)
    UPER_GADGET_FINALIZE
};

class RegionalValidityType {
    UPER_EXTENDABLE_GADGET
    UPER_ELEMENT(QVariant, value)
public:
    void decode(UPERDecoder &decoder);
};

class ReturnRouteDescriptionType {
    UPER_EXTENDABLE_GADGET
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

class RouteSectionType {
    UPER_GADGET
    // TODO
};

class SeriesDetailType {
    UPER_GADGET
};

class CardReferenceType {
    UPER_EXTENDABLE_GADGET
    UPER_ELEMENT_OPTIONAL(int, cardIssuerNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, cardIssuerIA5)
    UPER_ELEMENT_OPTIONAL(int, cardIdNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, cardIdIA5)
    UPER_ELEMENT_OPTIONAL(QString, cardName)
    UPER_ELEMENT_OPTIONAL(int, cardType)
    UPER_ELEMENT_OPTIONAL(int, leadingCardIdNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, leadingCardIdIA5)
    UPER_ELEMENT_OPTIONAL(int, trailingCardIdNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, trailingCardIdIA5)
    UPER_GADGET_FINALIZE
};

class TariffType {
    UPER_EXTENDABLE_GADGET
    UPER_ELEMENT_DEFAULT(int, numberOfPassengers, 1)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::PassengerType, passengerType)
    UPER_ELEMENT_OPTIONAL(int, ageBelow)
    UPER_ELEMENT_OPTIONAL(int, ageAbove)
    UPER_ELEMENT_OPTIONAL(QList<int>, travelerid)
    UPER_ELEMENT(bool, restrictedToCountryOfResidence)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::RouteSectionType, restrictedToRouteSection)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::SeriesDetailType, seriesDataDetails)
    UPER_ELEMENT_OPTIONAL(int, tariffIdNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, tariffIdIA5)
    UPER_ELEMENT_OPTIONAL(QString, tariffDesc)
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::CardReferenceType>, reductionCard)
    UPER_GADGET_FINALIZE
};

class VatDetailType {
    UPER_GADGET
    // TODO
};

class IncludedOpenTicketType {
    UPER_EXTENDABLE_GADGET
    // TODO
};

class LuggageRestrictionType {
    UPER_EXTENDABLE_GADGET
    // TODO
};

class OpenTicketData {
    UPER_EXTENDABLE_GADGET
    UPER_ELEMENT_OPTIONAL(int, referenceNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, referenceIA5)
    UPER_ELEMENT_OPTIONAL(int, productOwnerNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, productOwnerIA5)
    UPER_ELEMENT_OPTIONAL(int, productIdNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, productIdIA5)
    UPER_ELEMENT_OPTIONAL(int, extIssuerId)
    UPER_ELEMENT_OPTIONAL(int, issuerAutorizationId)
    UPER_ELEMENT(bool, returnIncluded)
    UPER_ELEMENT_DEFAULT(KItinerary::Fcb::CodeTableType, stationCodeTable, stationUIC)
    UPER_ELEMENT_OPTIONAL(int, fromStationNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, fromStationIA5)
    UPER_ELEMENT_OPTIONAL(int, toStationNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, toStationIA5)
    UPER_ELEMENT_OPTIONAL(QString, fromStationNameUTF8)
    UPER_ELEMENT_OPTIONAL(QString, toStationNameUTF8)
    UPER_ELEMENT_OPTIONAL(QString, validRegionDesc)
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::RegionalValidityType>, validRegion)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::ReturnRouteDescriptionType, returnDescription)
    UPER_ELEMENT_DEFAULT(int, validFromDay, 0)
    UPER_ELEMENT_OPTIONAL(int, validFromTime)
    UPER_ELEMENT_OPTIONAL(int, validFromUTCOffset)
    UPER_ELEMENT_DEFAULT(int, validUntilDay, 0)
    UPER_ELEMENT_OPTIONAL(int, validUntilTime)
    UPER_ELEMENT_OPTIONAL(int, validUntilUTCOffset)
    UPER_ELEMENT_OPTIONAL(QList<int>, activatedDay)
    UPER_ELEMENT_DEFAULT(KItinerary::Fcb::TravelClassType, classCode, second)
    UPER_ELEMENT_OPTIONAL(QByteArray, serviceLevel)
    UPER_ELEMENT_OPTIONAL(QList<int>, carrierNum)
    UPER_ELEMENT_OPTIONAL(QList<QByteArray>, carrierIA5)
    UPER_ELEMENT_OPTIONAL(QList<int>, includedServiceBrands)
    UPER_ELEMENT_OPTIONAL(QList<int>, excludedServiceBrands)
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::TariffType>, tariffs)
    UPER_ELEMENT_OPTIONAL(int, price)
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::VatDetailType>, vatDetail)
    UPER_ELEMENT_OPTIONAL(QString, infoText)
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::IncludedOpenTicketType>, includedAddOns)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::LuggageRestrictionType, luggage)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::ExtensionData, extension)
    UPER_GADGET_FINALIZE
};

class ValidityPeriodDetailType {
    // TODO
};

class PassData {
    UPER_EXTENDABLE_GADGET
    UPER_ELEMENT_OPTIONAL(int, referenceNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, referenceIA5)
    UPER_ELEMENT_OPTIONAL(int, productOwnerNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, productOwnerIA5)
    UPER_ELEMENT_OPTIONAL(int, productIdNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, productIdIA5)
    UPER_ELEMENT_OPTIONAL(int, passType)
    UPER_ELEMENT_OPTIONAL(QString, passDescription)
    UPER_ELEMENT_DEFAULT(KItinerary::Fcb::TravelClassType, classCode, second)
    UPER_ELEMENT_DEFAULT(int, validFromDay, 0)
    UPER_ELEMENT_OPTIONAL(int, validFromTime)
    UPER_ELEMENT_OPTIONAL(int, validFromUTCOffset)
    UPER_ELEMENT_DEFAULT(int, validUntilDay, 0)
    UPER_ELEMENT_OPTIONAL(int, validUntilTime)
    UPER_ELEMENT_OPTIONAL(int, validUntilUTCOffset)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::ValidityPeriodDetailType, validityPeriodDetails)
    UPER_ELEMENT_OPTIONAL(int, numberOfValidityDays)
    UPER_ELEMENT_OPTIONAL(int, numberOfPossibleTrips)
    UPER_ELEMENT_OPTIONAL(int, numberOfDaysOfTravel)
    UPER_ELEMENT_OPTIONAL(QList<int>, activatedDay)
    UPER_ELEMENT_OPTIONAL(QList<int>, countries)
    UPER_ELEMENT_OPTIONAL(QList<int>, includedCarrierNum)
    UPER_ELEMENT_OPTIONAL(QList<QByteArray>, includedCarrierIA5)
    UPER_ELEMENT_OPTIONAL(QList<int>, excludedCarrierNum)
    UPER_ELEMENT_OPTIONAL(QList<QByteArray>, excludedCarrierIA5)
    UPER_ELEMENT_OPTIONAL(QList<int>, includedServiceBrands)
    UPER_ELEMENT_OPTIONAL(QList<int>, excludedServiceBrands)
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::RegionalValidityType>, validRegion)
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::TariffType>, tariffs)
    UPER_ELEMENT_OPTIONAL(int, price)
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::VatDetailType>, vatDetail)
    UPER_ELEMENT_OPTIONAL(QString, infoText)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::ExtensionData, extension)
    UPER_GADGET_FINALIZE
};

class TokenType {
    UPER_GADGET
    UPER_ELEMENT_OPTIONAL(int, tokenProviderNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, tokenProviderIA5)
    UPER_ELEMENT_OPTIONAL(QByteArray, tokenSpecification)
    UPER_ELEMENT(QByteArray, token)
    UPER_GADGET_FINALIZE
};

class DocumentData {
    UPER_EXTENDABLE_GADGET
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::TokenType, token)
    UPER_ELEMENT(QVariant, ticket)
    UPER_GADGET_FINALIZE
};

class TicketLinkType {
    UPER_EXTENDABLE_GADGET
    // TODO
};

class ControlData {
    UPER_EXTENDABLE_GADGET
    UPER_ELEMENT_OPTIONAL(QList <KItinerary::Fcb::CardReferenceType>, identificationByCardReference)
    UPER_ELEMENT(bool, identificationByIdCard)
    UPER_ELEMENT(bool, identificationByPassportId)
    UPER_ELEMENT_OPTIONAL(int, identificationItem)
    UPER_ELEMENT(bool, passportValidationRequired)
    UPER_ELEMENT(bool, onlineValidationRequired)
    UPER_ELEMENT_OPTIONAL(int, randomDetailedValidationRequired)
    UPER_ELEMENT(bool, ageCheckRequired)
    UPER_ELEMENT(bool, reductionCardCheckRequired)
    UPER_ELEMENT_OPTIONAL(QString, infoText)
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::TicketLinkType>, includedTickets)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::ExtensionData, extension)
    UPER_GADGET_FINALIZE
};

class KITINERARY_EXPORT UicRailTicketData {
    UPER_EXTENDABLE_GADGET
    UPER_ELEMENT(KItinerary::Fcb::IssuingData, issuingDetail)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::TravelerData, travelerDetail)
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::DocumentData>, transportDocument)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::ControlData, controlDetail)
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::ExtensionData>, extension)
    UPER_GADGET_FINALIZE
public:
    UicRailTicketData();
    UicRailTicketData(const Uic9183Block &block);

    static constexpr const char RecordId[] = "U_FLEX";

private:
    Uic9183Block m_block;
};

}
}

Q_DECLARE_METATYPE(KItinerary::Fcb::ExtensionData)
Q_DECLARE_METATYPE(KItinerary::Fcb::GeoCoordinateType)
Q_DECLARE_METATYPE(KItinerary::Fcb::IssuingData)
Q_DECLARE_METATYPE(KItinerary::Fcb::TravelerType)
Q_DECLARE_METATYPE(KItinerary::Fcb::CustomerStatusType)
Q_DECLARE_METATYPE(KItinerary::Fcb::TravelerData)
Q_DECLARE_METATYPE(KItinerary::Fcb::RegionalValidityType)
Q_DECLARE_METATYPE(KItinerary::Fcb::ReturnRouteDescriptionType)
Q_DECLARE_METATYPE(KItinerary::Fcb::RouteSectionType)
Q_DECLARE_METATYPE(KItinerary::Fcb::SeriesDetailType)
Q_DECLARE_METATYPE(KItinerary::Fcb::TariffType)
Q_DECLARE_METATYPE(KItinerary::Fcb::VatDetailType)
Q_DECLARE_METATYPE(KItinerary::Fcb::IncludedOpenTicketType)
Q_DECLARE_METATYPE(KItinerary::Fcb::LuggageRestrictionType)
Q_DECLARE_METATYPE(KItinerary::Fcb::OpenTicketData)
Q_DECLARE_METATYPE(KItinerary::Fcb::ValidityPeriodDetailType)
Q_DECLARE_METATYPE(KItinerary::Fcb::PassData)
Q_DECLARE_METATYPE(KItinerary::Fcb::TokenType)
Q_DECLARE_METATYPE(KItinerary::Fcb::DocumentData)
Q_DECLARE_METATYPE(KItinerary::Fcb::CardReferenceType)
Q_DECLARE_METATYPE(KItinerary::Fcb::TicketLinkType)
Q_DECLARE_METATYPE(KItinerary::Fcb::ControlData)
Q_DECLARE_METATYPE(KItinerary::Fcb::UicRailTicketData)

#endif // KITINERARY_FCBTICKET_H
