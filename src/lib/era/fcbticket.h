/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_FCBTICKET_H
#define KITINERARY_FCBTICKET_H

#include <KItinerary/Uic9183Block>

#include "asn1/uperelement.h"

#include <QDateTime>
#include <QList>
#include <QVariant>

#include <bitset>

namespace KItinerary {

class UPERDecoder;

/** ERA Flexible Content Barcode (FCB) version 1.3
 *  @see ERA TAP TSI TD B.12 §11 and the corresponding ASN.1 definition
 *  @see https://github.com/UnionInternationalCheminsdeFer/UIC-barcode
 */
namespace Fcb {
namespace v13 {

Q_NAMESPACE

/** Generic extension data. */
class ExtensionData {
    UPER_GADGET
    UPER_ELEMENT(QByteArray, extensionId)
    UPER_ELEMENT(QByteArray, extensionData)
    UPER_GADGET_FINALIZE
};

enum GeoUnitType {
    microDegree = 0,
    tenthmilliDegree = 1,
    milliDegree = 2,
    centiDegree = 3,
    deciDegree = 4,
};
UPER_ENUM(GeoUnitType)

enum GeoCoordinateSystemType {
    wgs84 = 0,
    grs80 = 1,
};
UPER_ENUM(GeoCoordinateSystemType)

enum HemisphereLongitudeType {
    north = 0,
    south = 1,
};
UPER_ENUM(HemisphereLongitudeType)

enum HemisphereLatitudeType {
    east = 0,
    west = 1,
};
UPER_ENUM(HemisphereLatitudeType)

/** Geographic coordinate. */
class GeoCoordinateType {
    UPER_GADGET
    UPER_ELEMENT_DEFAULT(KItinerary::Fcb::v13::GeoUnitType, geoUnit, milliDegree)
    UPER_ELEMENT_DEFAULT(KItinerary::Fcb::v13::GeoCoordinateSystemType, coordinateSystem, wgs84)
    UPER_ELEMENT_DEFAULT(KItinerary::Fcb::v13::HemisphereLongitudeType, hemisphereLongitude, north)
    UPER_ELEMENT_DEFAULT(KItinerary::Fcb::v13::HemisphereLatitudeType, hemisphereLatitude, east)
    UPER_ELEMENT(int, longitude)
    UPER_ELEMENT(int, latitude)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::v13::GeoUnitType, accuracy)
    UPER_GADGET_FINALIZE
};

/** Delta encoding of a geographic coordinate. */
class DeltaCoordinate {
    UPER_GADGET
    UPER_ELEMENT(int, longitude)
    UPER_ELEMENT(int, latitude)
    UPER_GADGET_FINALIZE
};

/** Issuing information. */
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
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::v13::ExtensionData, extension)
    UPER_ELEMENT_OPTIONAL(int, issuedOnTrainNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, issuedOnTrainIA5)
    UPER_ELEMENT_OPTIONAL(int, issuedOnLine)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::v13::GeoCoordinateType, pointOfSale)
    UPER_GADGET_FINALIZE

    Q_PROPERTY(QDateTime issueingDateTime READ issueingDateTime)
public:
    [[nodiscard]] QDateTime issueingDateTime() const;
};

enum GenderType {
    unspecified = 0,
    female = 1,
    male = 2,
    other = 3,
};
UPER_EXTENABLE_ENUM(GenderType)

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
UPER_EXTENABLE_ENUM(PassengerType)

/** Customer status information. */
class CustomerStatusType {
    UPER_GADGET
    UPER_ELEMENT_OPTIONAL(int, statusProviderNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, statusProviderIA5)
    UPER_ELEMENT_OPTIONAL(int, customerStatus)
    UPER_ELEMENT_OPTIONAL(QByteArray, customerStatusDescr)
    UPER_GADGET_FINALIZE
};

/** Information about a single traveler. */
class TravelerType {
    UPER_EXTENDABLE_GADGET
    UPER_ELEMENT_OPTIONAL(QString, firstName)
    UPER_ELEMENT_OPTIONAL(QString, secondName)
    UPER_ELEMENT_OPTIONAL(QString, lastName)
    UPER_ELEMENT_OPTIONAL(QByteArray, idCard)
    UPER_ELEMENT_OPTIONAL(QByteArray, passportId)
    UPER_ELEMENT_OPTIONAL(QByteArray, title)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::v13::GenderType, gender)
    UPER_ELEMENT_OPTIONAL(QByteArray, customerIdIA5)
    UPER_ELEMENT_OPTIONAL(int, customerIdNum)
    UPER_ELEMENT_OPTIONAL(int, yearOfBirth)
    UPER_ELEMENT_OPTIONAL(int, dayOfBirth)
    UPER_ELEMENT(bool, ticketHolder)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::v13::PassengerType, passengerType)
    UPER_ELEMENT_OPTIONAL(bool, passengerWithReducedMobility)
    UPER_ELEMENT_OPTIONAL(int, countryOfResidence)
    UPER_ELEMENT_OPTIONAL(int, countryOfPassport)
    UPER_ELEMENT_OPTIONAL(int, countryOfIdCard)
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::v13::CustomerStatusType>, status)
    UPER_GADGET_FINALIZE
};

/** A set of traverlers. */
class TravelerData {
    UPER_EXTENDABLE_GADGET
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::v13::TravelerType>, traveler)
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
UPER_ENUM(CodeTableType)

/** Reference to a specific train journey. */
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

public:
    Q_INVOKABLE [[nodiscard]] QDateTime departureDateTime(const QDateTime &issueingDateTime) const;
};

/** A set of via stations. */
class ViaStationType {
    UPER_EXTENDABLE_GADGET
    UPER_ELEMENT_DEFAULT(KItinerary::Fcb::v13::CodeTableType, stationCodeTable, stationUIC)
    UPER_ELEMENT_OPTIONAL(int, stationNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, stationIA5)
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::v13::ViaStationType>, alternativeRoutes)
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::v13::ViaStationType>, route)
    UPER_ELEMENT(bool, border)
    UPER_ELEMENT_OPTIONAL(QList<int>, carrierNum)
    UPER_ELEMENT_OPTIONAL(QList<QByteArray>, carrierIA5)
    UPER_ELEMENT_OPTIONAL(int, seriesId)
    UPER_ELEMENT_OPTIONAL(int, routeId)
    UPER_GADGET_FINALIZE
};

/** A tariff zone. */
class ZoneType {
    UPER_EXTENDABLE_GADGET
    UPER_ELEMENT_OPTIONAL(int, carrierNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, carrierIA5)
    UPER_ELEMENT_DEFAULT(KItinerary::Fcb::v13::CodeTableType, stationCodeTable, stationUIC)
    UPER_ELEMENT_OPTIONAL(int, entryStationNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, entryStationIA5)
    UPER_ELEMENT_OPTIONAL(int, terminatingStationNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, terminatingStationIA5)
    UPER_ELEMENT_OPTIONAL(int, city)
    UPER_ELEMENT_OPTIONAL(QList<int>, zoneId)
    UPER_ELEMENT_OPTIONAL(QByteArray, binaryZoneId)
    UPER_ELEMENT_OPTIONAL(QByteArray, nutsCode)
    UPER_GADGET_FINALIZE
};

/** */
class LineType {
    UPER_EXTENDABLE_GADGET
    UPER_ELEMENT_OPTIONAL(int, carrierNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, carrierIA5)
    UPER_ELEMENT_OPTIONAL(QList<int>, lineId)
    UPER_ELEMENT_DEFAULT(KItinerary::Fcb::v13::CodeTableType, stationCodeTable, stationUIC)
    UPER_ELEMENT_OPTIONAL(int, entryStationNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, entryStationIA5)
    UPER_ELEMENT_OPTIONAL(int, terminatingStationNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, terminatingStationIA5)
    UPER_ELEMENT_OPTIONAL(int, city)
    UPER_ELEMENT_OPTIONAL(QByteArray, binaryZoneId)
    UPER_GADGET_FINALIZE
};

/** A geographic polygon. */
class PolygoneType {
    UPER_GADGET
    UPER_ELEMENT(KItinerary::Fcb::v13::GeoCoordinateType, firstEdge)
    UPER_ELEMENT(QList<KItinerary::Fcb::v13::DeltaCoordinate>, edges)
    UPER_GADGET_FINALIZE
};

/** Regional validity information.
 *  Can be one of:
 *  - TrainLinkType
 *  - ViaStationType
 *  - ZoneType
 *  - LineType
 *  - PolygoneType
 */
class RegionalValidityType {
    UPER_EXTENDABLE_GADGET
    UPER_ELEMENT(QVariant, value)
    UPER_GADGET_FINALIZE
};

class ReturnRouteDescriptionType {
    UPER_EXTENDABLE_GADGET
    UPER_ELEMENT_OPTIONAL(int, fromStationNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, fromStationIA5)
    UPER_ELEMENT_OPTIONAL(int, toStationNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, toStationIA5)
    UPER_ELEMENT_OPTIONAL(QString, fromStationNameUTF8)
    UPER_ELEMENT_OPTIONAL(QString, toStationNameUTF8)
    UPER_ELEMENT_OPTIONAL(QString, validReturnRegionDesc)
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::v13::RegionalValidityType>, validReturnRegion)
    UPER_GADGET_FINALIZE
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
UPER_EXTENABLE_ENUM(TravelClassType)

class RouteSectionType {
    UPER_GADGET
    UPER_ELEMENT_DEFAULT(KItinerary::Fcb::v13::CodeTableType, stationCodeTable, stationUIC)
    UPER_ELEMENT_OPTIONAL(int, fromStationNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, fromStationIA5)
    UPER_ELEMENT_OPTIONAL(int, toStationNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, toStationIA5)
    UPER_ELEMENT_OPTIONAL(QString, fromStationNameUTF8)
    UPER_ELEMENT_OPTIONAL(QString, toStationNameUTF8)
    UPER_GADGET_FINALIZE
};

class SeriesDetailType {
    UPER_GADGET
    UPER_ELEMENT_OPTIONAL(int, supplyingCarrier)
    UPER_ELEMENT_OPTIONAL(int, offerIdentification)
    UPER_ELEMENT_OPTIONAL(int, series)
    UPER_GADGET_FINALIZE
};

/** Customer card information. */
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

enum ServiceType {
    seat = 0,
    couchette = 1,
    berth = 2,
    carcarriage = 3,
};
UPER_ENUM(ServiceType)

/** Seat information. */
class PlacesType {
    UPER_GADGET
    UPER_ELEMENT_OPTIONAL(QByteArray, coach)
    UPER_ELEMENT_OPTIONAL(QByteArray, placeString)
    UPER_ELEMENT_OPTIONAL(QString, placeDescription)
    UPER_ELEMENT_OPTIONAL(QList<QByteArray>, placeIA5)
    UPER_ELEMENT_OPTIONAL(QList<int>, placeNum)
    UPER_GADGET_FINALIZE
};

enum class CompartmentPositionType {
    unspecified = 0, // TODO ambiguous, therefore using enum class for now, but that's not going to work in QML
    upperLevel = 1,
    lowerLevel = 2,
};
UPER_ENUM(CompartmentPositionType)

/** Compartment information. */
class CompartmentDetailsType {
    UPER_EXTENDABLE_GADGET
    UPER_ELEMENT_OPTIONAL(int, coachType)
    UPER_ELEMENT_OPTIONAL(int, compartmentType)
    UPER_ELEMENT_OPTIONAL(int, specialAllocation)
    UPER_ELEMENT_OPTIONAL(QString, coachTypeDescr)
    UPER_ELEMENT_OPTIONAL(QString, compartmentTypeDescr)
    UPER_ELEMENT_OPTIONAL(QString, specialAllocationDescr)
    UPER_ELEMENT_DEFAULT(KItinerary::Fcb::v13::CompartmentPositionType, position, CompartmentPositionType::unspecified)
    UPER_GADGET_FINALIZE
};

enum BerthTypeType {
    single = 0,
    special = 1,
    doubleBerth = 2, // "double" in the spec, but that conflicts
    t2 = 3,
    t3 = 4,
    t4 = 5,
};
UPER_ENUM(BerthTypeType)

enum class CompartmentGenderType {
    unspecified = 0, // TODO see CompartmentPositionType
    family = 1,
    female = 2,
    male = 3,
    mixed = 4,
};
UPER_EXTENABLE_ENUM(CompartmentGenderType)

/** Berth information. */
class BerthDetailData {
    UPER_EXTENDABLE_GADGET
    UPER_ELEMENT(KItinerary::Fcb::v13::BerthTypeType, berthType)
    UPER_ELEMENT(int, numberOfBerths)
    UPER_ELEMENT_DEFAULT(KItinerary::Fcb::v13::CompartmentGenderType, gender, CompartmentGenderType::family)
    UPER_GADGET_FINALIZE
};

/** Tariff information. */
class TariffType {
    UPER_EXTENDABLE_GADGET
    UPER_ELEMENT_DEFAULT(int, numberOfPassengers, 1)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::v13::PassengerType, passengerType)
    UPER_ELEMENT_OPTIONAL(int, ageBelow)
    UPER_ELEMENT_OPTIONAL(int, ageAbove)
    UPER_ELEMENT_OPTIONAL(QList<int>, travelerid)
    UPER_ELEMENT(bool, restrictedToCountryOfResidence)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::v13::RouteSectionType, restrictedToRouteSection)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::v13::SeriesDetailType, seriesDataDetails)
    UPER_ELEMENT_OPTIONAL(int, tariffIdNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, tariffIdIA5)
    UPER_ELEMENT_OPTIONAL(QString, tariffDesc)
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::v13::CardReferenceType>, reductionCard)
    UPER_GADGET_FINALIZE
};

enum PriceTypeType {
    noPrice = 0,
    reservationFee = 1,
    supplement = 2,
    travelPrice = 3,
};
UPER_ENUM(PriceTypeType)

/** VAT information. */
class VatDetailType {
    UPER_GADGET
    UPER_ELEMENT(int, country)
    UPER_ELEMENT(int, percentage)
    UPER_ELEMENT_OPTIONAL(int, amount)
    UPER_ELEMENT_OPTIONAL(QByteArray, vatId)
    UPER_GADGET_FINALIZE
};

/** Open tickets included into a reservation. */
class IncludedOpenTicketType {
    UPER_EXTENDABLE_GADGET
    UPER_ELEMENT_OPTIONAL(int, productOwnerNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, productOwnerIA5)
    UPER_ELEMENT_OPTIONAL(int, productIdNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, productIdIA5)
    UPER_ELEMENT_OPTIONAL(int, externalIssuerId)
    UPER_ELEMENT_OPTIONAL(int, issuerAutorizationId)
    UPER_ELEMENT_DEFAULT(KItinerary::Fcb::v13::CodeTableType, stationCodeTable, stationUIC)
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::v13::RegionalValidityType>, validRegion)
    UPER_ELEMENT_DEFAULT(int, validFromDay, 0)
    UPER_ELEMENT_OPTIONAL(int, validFromTime)
    UPER_ELEMENT_OPTIONAL(int, validFromUTCOffset)
    UPER_ELEMENT_DEFAULT(int, validUntilDay, 0)
    UPER_ELEMENT_OPTIONAL(int, validUntilTime)
    UPER_ELEMENT_OPTIONAL(int, validUntilUTCOffset)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::v13::TravelClassType, classCode)
    UPER_ELEMENT_OPTIONAL(QByteArray, serviceLevel)
    UPER_ELEMENT_OPTIONAL(QList<int>, carrierNum)
    UPER_ELEMENT_OPTIONAL(QList<QByteArray>, carrierIA5)
    UPER_ELEMENT_OPTIONAL(QList<int>, includedServiceBrands)
    UPER_ELEMENT_OPTIONAL(QList<int>, excludedServiceBrands)
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::v13::TariffType>, tariffs)
    UPER_ELEMENT_OPTIONAL(QString, infoText)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::v13::ExtensionData, extension)
    UPER_GADGET_FINALIZE
};

/** Luggage information. */
class RegisteredLuggageType {
    UPER_EXTENDABLE_GADGET
    UPER_ELEMENT_OPTIONAL(QByteArray, registrationId)
    UPER_ELEMENT_OPTIONAL(int, maxWeight)
    UPER_ELEMENT_OPTIONAL(int, maxSize)
    UPER_GADGET_FINALIZE
};

/** Luggage restriction information. */
class LuggageRestrictionType {
    UPER_EXTENDABLE_GADGET
    UPER_ELEMENT_DEFAULT(int, maxHandLuggagePieces, 3)
    UPER_ELEMENT_DEFAULT(int, maxNonHandLuggagePieces, 1)
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::v13::RegisteredLuggageType>, registeredLuggage)
    UPER_GADGET_FINALIZE
};

/** Reservation document (IRT, RES). */
class ReservationData {
    UPER_EXTENDABLE_GADGET
    UPER_ELEMENT_OPTIONAL(int, trainNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, trainIA5)
    UPER_ELEMENT_DEFAULT(int, departureDate, 0)
    UPER_ELEMENT_OPTIONAL(QByteArray, referenceIA5)
    UPER_ELEMENT_OPTIONAL(int, referenceNum)
    UPER_ELEMENT_OPTIONAL(int, productOwnerNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, productOwnerIA5)
    UPER_ELEMENT_OPTIONAL(int, productIdNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, productIdIA5)
    UPER_ELEMENT_OPTIONAL(int, serviceBrand)
    UPER_ELEMENT_OPTIONAL(QString, serviceBrandAbrUTF8)
    UPER_ELEMENT_OPTIONAL(QString, serviceBrandNameUTF8)
    UPER_ELEMENT_DEFAULT(KItinerary::Fcb::v13::ServiceType, service, seat)
    UPER_ELEMENT_DEFAULT(KItinerary::Fcb::v13::CodeTableType, stationCodeTable, stationUICReservation)
    UPER_ELEMENT_OPTIONAL(int, fromStationNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, fromStationIA5)
    UPER_ELEMENT_OPTIONAL(int, toStationNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, toStationIA5)
    UPER_ELEMENT_OPTIONAL(QString, fromStationNameUTF8)
    UPER_ELEMENT_OPTIONAL(QString, toStationNameUTF8)
    UPER_ELEMENT(int, departureTime)
    UPER_ELEMENT_OPTIONAL(int, departureUTCOffset)
    UPER_ELEMENT_DEFAULT(int, arrivalDate, 0)
    UPER_ELEMENT_OPTIONAL(int, arrivalTime)
    UPER_ELEMENT_OPTIONAL(int, arrivalUTCOffset)
    UPER_ELEMENT_OPTIONAL(QList<int>, carrierNum)
    UPER_ELEMENT_OPTIONAL(QList<QByteArray>, carrierIA5)
    UPER_ELEMENT_DEFAULT(KItinerary::Fcb::v13::TravelClassType, classCode, second)
    UPER_ELEMENT_OPTIONAL(QByteArray, serviceLevel)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::v13::PlacesType, places)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::v13::PlacesType, additionalPlaces)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::v13::PlacesType, bicyclePlaces)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::v13::CompartmentDetailsType, compartmentDetails)
    UPER_ELEMENT_DEFAULT(int, numberOfOverbooked, 0)
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::v13::BerthDetailData>, berth)
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::v13::TariffType>, tariffs) // "tariff" in the official ASN.1 spec, but that inconsistent with the other document types
    UPER_ELEMENT_DEFAULT(KItinerary::Fcb::v13::PriceTypeType, priceType, travelPrice)
    UPER_ELEMENT_OPTIONAL(int, price)
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::v13::VatDetailType>, vatDetail)
    UPER_ELEMENT_DEFAULT(int, typeOfSupplement, 0)
    UPER_ELEMENT_DEFAULT(int, numberOfSupplements, 0)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::v13::LuggageRestrictionType, luggage)
    UPER_ELEMENT_OPTIONAL(QString, infoText)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::v13::ExtensionData, extension)
    UPER_GADGET_FINALIZE

public:
    Q_INVOKABLE [[nodiscard]] QDateTime departureDateTime(const QDateTime &issueingDateTime) const;
    Q_INVOKABLE [[nodiscard]] QDateTime arrivalDateTime(const QDateTime &issueingDateTime) const;
};

enum RoofRackType {
    norack = 0,
    roofRailing = 1,
    luggageRack = 2,
    skiRack = 3,
    boxRack = 4,
    rackWithOneBox = 5,
    rackWithTwoBoxes = 6,
    bicycleRack = 7,
    otherRack = 8,
};
UPER_EXTENABLE_ENUM(RoofRackType)

enum class LoadingDeckType {
    unspecified = 0, // TODO see above
    upper = 1,
    lower = 2,
};
UPER_ENUM(LoadingDeckType)

/** Car carriage reservation document. */
class CarCarriageReservationData {
    UPER_EXTENDABLE_GADGET
    UPER_ELEMENT_OPTIONAL(int, trainNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, trainIA5)
    UPER_ELEMENT_DEFAULT(int, beginLoadingDate, 0)
    UPER_ELEMENT_OPTIONAL(int, beginLoadingTime)
    UPER_ELEMENT_OPTIONAL(int, endLoadingTime)
    UPER_ELEMENT_OPTIONAL(int, loadingUTCOffset)
    UPER_ELEMENT_OPTIONAL(QByteArray, referenceIA5)
    UPER_ELEMENT_OPTIONAL(int, referenceNum)
    UPER_ELEMENT_OPTIONAL(int, productOwnerNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, productOwnerIA5)
    UPER_ELEMENT_OPTIONAL(int, productIdNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, productIdIA5)
    UPER_ELEMENT_OPTIONAL(int, serviceBrand)
    UPER_ELEMENT_OPTIONAL(QString, serviceBrandAbrUTF8)
    UPER_ELEMENT_OPTIONAL(QString, serviceBrandNameUTF8)
    UPER_ELEMENT_DEFAULT(KItinerary::Fcb::v13::CodeTableType, stationCodeTable, stationUICReservation)
    UPER_ELEMENT_OPTIONAL(int, fromStationNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, fromStationIA5)
    UPER_ELEMENT_OPTIONAL(int, toStationNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, toStationIA5)
    UPER_ELEMENT_OPTIONAL(QString, fromStationNameUTF8)
    UPER_ELEMENT_OPTIONAL(QString, toStationNameUTF8)
    UPER_ELEMENT_OPTIONAL(QByteArray, coach)
    UPER_ELEMENT_OPTIONAL(QByteArray, place)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::v13::CompartmentDetailsType, compartmentDetails)
    UPER_ELEMENT_OPTIONAL(QByteArray, numberPlate)
    UPER_ELEMENT_OPTIONAL(QByteArray, trailerPlate)
    UPER_ELEMENT(int, carCategory)
    UPER_ELEMENT_OPTIONAL(int, boatCategory)
    UPER_ELEMENT(bool, textileRoof)
    UPER_ELEMENT_DEFAULT(KItinerary::Fcb::v13::RoofRackType, roofRackType, norack)
    UPER_ELEMENT_OPTIONAL(int, roofRackHeight)
    UPER_ELEMENT_OPTIONAL(int, attachedBoats)
    UPER_ELEMENT_OPTIONAL(int, attachedBicycles)
    UPER_ELEMENT_OPTIONAL(int, attachedSurfboards)
    UPER_ELEMENT_OPTIONAL(int, loadingListEntry)
    UPER_ELEMENT_DEFAULT(KItinerary::Fcb::v13::LoadingDeckType, loadingDeck, LoadingDeckType::upper)
    UPER_ELEMENT_OPTIONAL(QList<int>, carrierNum)
    UPER_ELEMENT_OPTIONAL(QList<QByteArray>, carrierIA5)
    UPER_ELEMENT(KItinerary::Fcb::v13::TariffType, tariff)
    UPER_ELEMENT_DEFAULT(KItinerary::Fcb::v13::PriceTypeType, priceType, travelPrice)
    UPER_ELEMENT_OPTIONAL(int, price)
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::v13::VatDetailType>, vatDetail)
    UPER_ELEMENT_OPTIONAL(QString, infoText)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::v13::ExtensionData, extension)
    UPER_GADGET_FINALIZE
};

/** Open ticket document (NRT). */
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
    UPER_ELEMENT_DEFAULT(KItinerary::Fcb::v13::CodeTableType, stationCodeTable, stationUIC)
    UPER_ELEMENT_OPTIONAL(int, fromStationNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, fromStationIA5)
    UPER_ELEMENT_OPTIONAL(int, toStationNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, toStationIA5)
    UPER_ELEMENT_OPTIONAL(QString, fromStationNameUTF8)
    UPER_ELEMENT_OPTIONAL(QString, toStationNameUTF8)
    UPER_ELEMENT_OPTIONAL(QString, validRegionDesc)
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::v13::RegionalValidityType>, validRegion)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::v13::ReturnRouteDescriptionType, returnDescription)
    UPER_ELEMENT_DEFAULT(int, validFromDay, 0)
    UPER_ELEMENT_OPTIONAL(int, validFromTime)
    UPER_ELEMENT_OPTIONAL(int, validFromUTCOffset)
    UPER_ELEMENT_DEFAULT(int, validUntilDay, 0)
    UPER_ELEMENT_OPTIONAL(int, validUntilTime)
    UPER_ELEMENT_OPTIONAL(int, validUntilUTCOffset)
    UPER_ELEMENT_OPTIONAL(QList<int>, activatedDay)
    UPER_ELEMENT_DEFAULT(KItinerary::Fcb::v13::TravelClassType, classCode, second)
    UPER_ELEMENT_OPTIONAL(QByteArray, serviceLevel)
    UPER_ELEMENT_OPTIONAL(QList<int>, carrierNum)
    UPER_ELEMENT_OPTIONAL(QList<QByteArray>, carrierIA5)
    UPER_ELEMENT_OPTIONAL(QList<int>, includedServiceBrands)
    UPER_ELEMENT_OPTIONAL(QList<int>, excludedServiceBrands)
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::v13::TariffType>, tariffs)
    UPER_ELEMENT_OPTIONAL(int, price)
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::v13::VatDetailType>, vatDetail)
    UPER_ELEMENT_OPTIONAL(QString, infoText)
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::v13::IncludedOpenTicketType>, includedAddOns)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::v13::LuggageRestrictionType, luggage)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::v13::ExtensionData, extension)
    UPER_GADGET_FINALIZE

public:
    Q_INVOKABLE [[nodiscard]] QDateTime validFrom(const QDateTime &issueingDateTime) const;
    Q_INVOKABLE [[nodiscard]] QDateTime validUntil(const QDateTime &issueingDateTime) const;
};

/** Time range. */
class TimeRangeType {
    UPER_GADGET
    UPER_ELEMENT(int, fromTime)
    UPER_ELEMENT(int, untilTime)
    UPER_GADGET_FINALIZE
};

/** Validity time period. */
class ValidityPeriodType {
    UPER_GADGET
    UPER_ELEMENT_DEFAULT(int, validFromDay, 0)
    UPER_ELEMENT_OPTIONAL(int, validFromTime)
    UPER_ELEMENT_OPTIONAL(int, validFromUTCOffset)
    UPER_ELEMENT_DEFAULT(int, validUntilDay, 0)
    UPER_ELEMENT_OPTIONAL(int, validUntilTime)
    UPER_ELEMENT_OPTIONAL(int, validUntilUTCOffset)
    UPER_GADGET_FINALIZE
};

/** Set of validity time period. */
class ValidityPeriodDetailType {
    UPER_GADGET
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::v13::ValidityPeriodDetailType>, validityPeriod)
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::v13::TimeRangeType>, excludedTimeRange)
    UPER_GADGET_FINALIZE
};

/** Rail pass document (RPT). */
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
    UPER_ELEMENT_DEFAULT(KItinerary::Fcb::v13::TravelClassType, classCode, second)
    UPER_ELEMENT_DEFAULT(int, validFromDay, 0)
    UPER_ELEMENT_OPTIONAL(int, validFromTime)
    UPER_ELEMENT_OPTIONAL(int, validFromUTCOffset)
    UPER_ELEMENT_DEFAULT(int, validUntilDay, 0)
    UPER_ELEMENT_OPTIONAL(int, validUntilTime)
    UPER_ELEMENT_OPTIONAL(int, validUntilUTCOffset)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::v13::ValidityPeriodDetailType, validityPeriodDetails)
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
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::v13::RegionalValidityType>, validRegion)
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::v13::TariffType>, tariffs)
    UPER_ELEMENT_OPTIONAL(int, price)
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::v13::VatDetailType>, vatDetail)
    UPER_ELEMENT_OPTIONAL(QString, infoText)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::v13::ExtensionData, extension)
    UPER_GADGET_FINALIZE

public:
    Q_INVOKABLE [[nodiscard]] QDateTime validFrom(const QDateTime &issueingDateTime) const;
    Q_INVOKABLE [[nodiscard]] QDateTime validUntil(const QDateTime &issueingDateTime) const;
};

/** Voucher document. */
class VoucherData {
    UPER_EXTENDABLE_GADGET
    UPER_ELEMENT_OPTIONAL(QByteArray, referenceIA5)
    UPER_ELEMENT_OPTIONAL(int, referenceNum)
    UPER_ELEMENT_OPTIONAL(int, productOwnerNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, productOwnerIA5)
    UPER_ELEMENT_OPTIONAL(int, productIdNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, productIdIA5)
    UPER_ELEMENT(int, validFromYear)
    UPER_ELEMENT(int, validFromDay)
    UPER_ELEMENT(int, validUntilYear)
    UPER_ELEMENT(int, validUntilDay)
    UPER_ELEMENT_DEFAULT(int, value, 0)
    UPER_ELEMENT_OPTIONAL(int, type)
    UPER_ELEMENT_OPTIONAL(QString, infoText)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::v13::ExtensionData, extension)
    UPER_GADGET_FINALIZE
};

/** Customer card document. */
class CustomerCardData {
    UPER_EXTENDABLE_GADGET
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::v13::TravelerType, customer)
    UPER_ELEMENT_OPTIONAL(QByteArray, cardIdIA5)
    UPER_ELEMENT_OPTIONAL(int, cardIdNum)
    UPER_ELEMENT(int, validFromYear)
    UPER_ELEMENT_OPTIONAL(int, validFromDay)
    UPER_ELEMENT_DEFAULT(int, validUntilYear, 0)
    UPER_ELEMENT_OPTIONAL(int, validUntilDay)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::v13::TravelClassType, classCode)
    UPER_ELEMENT_OPTIONAL(int, cardType)
    UPER_ELEMENT_OPTIONAL(QString, cardTypeDescr)
    UPER_ELEMENT_OPTIONAL(int, customerStatus)
    UPER_ELEMENT_OPTIONAL(QByteArray, customerStatusDescr)
    UPER_ELEMENT_OPTIONAL(QList<int>, includedServices)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::v13::ExtensionData, extension)
    UPER_GADGET_FINALIZE

    Q_PROPERTY(QDate validFrom READ validFrom)
    Q_PROPERTY(QDate validUntil READ validUntil)
public:
    [[nodiscard]] QDate validFrom() const;
    [[nodiscard]] QDate validUntil() const;
};

/** Countermark document. */
class CountermarkData {
    UPER_EXTENDABLE_GADGET
    UPER_ELEMENT_OPTIONAL(QByteArray, referenceIA5)
    UPER_ELEMENT_OPTIONAL(int, referenceNum)
    UPER_ELEMENT_OPTIONAL(int, productOwnerNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, productOwnerIA5)
    UPER_ELEMENT_OPTIONAL(int, productIdNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, productIdIA5)
    UPER_ELEMENT_OPTIONAL(QByteArray, ticketReferenceIA5)
    UPER_ELEMENT_OPTIONAL(int, ticketReferenceNum)
    UPER_ELEMENT(int, numberOfCountermark)
    UPER_ELEMENT(int, totalOfCountermarks)
    UPER_ELEMENT(QString, groupName)
    UPER_ELEMENT_DEFAULT(KItinerary::Fcb::v13::CodeTableType, stationCodeTable, stationUIC)
    UPER_ELEMENT_OPTIONAL(int, fromStationNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, fromStationIA5)
    UPER_ELEMENT_OPTIONAL(int, toStationNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, toStationIA5)
    UPER_ELEMENT_OPTIONAL(QString, fromStationNameUTF8)
    UPER_ELEMENT_OPTIONAL(QString, toStationNameUTF8)
    UPER_ELEMENT_OPTIONAL(QString, validRegionDesc)
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::v13::RegionalValidityType>, validRegion)
    UPER_ELEMENT(bool, returnIncluded)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::v13::ReturnRouteDescriptionType, returnDescription)
    UPER_ELEMENT_DEFAULT(int, validFromDay, 0)
    UPER_ELEMENT_OPTIONAL(int, validFromTime)
    UPER_ELEMENT_OPTIONAL(int, validFromUTCOffset)
    UPER_ELEMENT_DEFAULT(int, validUntilDay, 0)
    UPER_ELEMENT_OPTIONAL(int, validUntilTime)
    UPER_ELEMENT_OPTIONAL(int, validUntilUTCOffset)
    UPER_ELEMENT_DEFAULT(KItinerary::Fcb::v13::TravelClassType, classCode, second)
    UPER_ELEMENT_OPTIONAL(QList<int>, carrierNum)
    UPER_ELEMENT_OPTIONAL(QList<QByteArray>, carrierIA5)
    UPER_ELEMENT_OPTIONAL(QList<int>, includedServiceBrands)
    UPER_ELEMENT_OPTIONAL(QList<int>, excludedServiceBrands)
    UPER_ELEMENT_OPTIONAL(QString, infoText)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::v13::ExtensionData, extension)
    UPER_GADGET_FINALIZE
};

/** Parking ground document. */
class ParkingGroundData {
    UPER_EXTENDABLE_GADGET
    UPER_ELEMENT_OPTIONAL(QByteArray, referenceIA5)
    UPER_ELEMENT_OPTIONAL(int, referenceNum)
    UPER_ELEMENT(QByteArray, parkingGroundId)
    UPER_ELEMENT(int, fromParkingDate)
    UPER_ELEMENT_DEFAULT(int, untilParkingDate, 0)
    UPER_ELEMENT_OPTIONAL(int, productOwnerNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, productOwnerIA5)
    UPER_ELEMENT_OPTIONAL(int, productIdNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, productIdIA5)
    UPER_ELEMENT_OPTIONAL(QByteArray, accessCode)
    UPER_ELEMENT(QString, location)
    UPER_ELEMENT_DEFAULT(KItinerary::Fcb::v13::CodeTableType, stationCodeTable, stationUIC)
    UPER_ELEMENT_OPTIONAL(int, stationNum)
    UPER_ELEMENT_OPTIONAL(QString, stationIA5) // yep, actually UTF8String rather than IA5String
    UPER_ELEMENT_OPTIONAL(QString, specialInformation)
    UPER_ELEMENT_OPTIONAL(QString, entryTrack)
    UPER_ELEMENT_OPTIONAL(QByteArray, numberPlate)
    UPER_ELEMENT_OPTIONAL(int, price)
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::v13::VatDetailType>, vatDetail)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::v13::ExtensionData, extension)
    UPER_GADGET_FINALIZE
};

/** FIP ticket document. */
class FIPTicketData {
    UPER_EXTENDABLE_GADGET
    UPER_ELEMENT_OPTIONAL(QByteArray, referenceIA5)
    UPER_ELEMENT_OPTIONAL(int, referenceNum)
    UPER_ELEMENT_OPTIONAL(int, productOwnerNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, productOwnerIA5)
    UPER_ELEMENT_OPTIONAL(int, productIdNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, productIdIA5)
    UPER_ELEMENT_DEFAULT(int, validFromDay, 0)
    UPER_ELEMENT_DEFAULT(int, validUntilDay, 0)
    UPER_ELEMENT_OPTIONAL(QList<int>, activatedDay)
    UPER_ELEMENT_OPTIONAL(QList<int>, carrierNum)
    UPER_ELEMENT_OPTIONAL(QList<QByteArray>, carrierIA5)
    UPER_ELEMENT(int, numberOfTravelDays)
    UPER_ELEMENT(bool, includesSupplements)
    UPER_ELEMENT_DEFAULT(KItinerary::Fcb::v13::TravelClassType, classCode, second)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::v13::ExtensionData, extension)
    UPER_GADGET_FINALIZE
};

/** Station passage document. */
class StationPassageData {
    UPER_EXTENDABLE_GADGET
    UPER_ELEMENT_OPTIONAL(QByteArray, referenceIA5)
    UPER_ELEMENT_OPTIONAL(int, referenceNum)
    UPER_ELEMENT_OPTIONAL(int, productOwnerNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, productOwnerIA5)
    UPER_ELEMENT_OPTIONAL(int, productIdNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, productIdIA5)
    UPER_ELEMENT_OPTIONAL(QString, productName)
    UPER_ELEMENT_DEFAULT(KItinerary::Fcb::v13::CodeTableType, stationCodeTable, stationUIC)
    UPER_ELEMENT_OPTIONAL(QList<int>, stationNum)
    UPER_ELEMENT_OPTIONAL(QList<QByteArray>, stationIA5)
    UPER_ELEMENT_OPTIONAL(QList<QString>, stationNameUTF8)
    UPER_ELEMENT_OPTIONAL(QList<int>, areaCodeNum)
    UPER_ELEMENT_OPTIONAL(QList<QByteArray>, areaCodeIA5)
    UPER_ELEMENT_OPTIONAL(QList<QString>, areaNameUTF8)
    UPER_ELEMENT(int, validFromDay)
    UPER_ELEMENT_OPTIONAL(int, validFromTime)
    UPER_ELEMENT_OPTIONAL(int, validFromUTCOffset)
    UPER_ELEMENT_DEFAULT(int, validUntilDay, 0)
    UPER_ELEMENT_OPTIONAL(int, validUntilTime)
    UPER_ELEMENT_OPTIONAL(int, validUntilUTCOffset)
    UPER_ELEMENT_OPTIONAL(int, numberOfDaysValid)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::v13::ExtensionData, extension)
    UPER_GADGET_FINALIZE
};

enum ConfirmationType {
    trainDelayConfirmation = 0,
    travelerDelayConfirmation = 1,
    trainLinkedTicketDelay = 2,
};
UPER_EXTENABLE_ENUM(ConfirmationType)

enum TicketType {
    openTicket = 0,
    pass = 1,
    reservation = 2,
    carCarriageReservation = 3,
};
UPER_EXTENABLE_ENUM(TicketType)

enum LinkMode {
    issuedTogether = 0,
    onlyValidInCombination = 1,
};
UPER_EXTENABLE_ENUM(LinkMode)

/** */
class TicketLinkType {
    UPER_EXTENDABLE_GADGET
    UPER_ELEMENT_OPTIONAL(QByteArray, referenceIA5)
    UPER_ELEMENT_OPTIONAL(int, referenceNum)
    UPER_ELEMENT_OPTIONAL(QString, issuerName)
    UPER_ELEMENT_OPTIONAL(QByteArray, issuerPNR)
    UPER_ELEMENT_OPTIONAL(int, productOwnerNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, productOwnerIA5)
    UPER_ELEMENT_DEFAULT(KItinerary::Fcb::v13::TicketType, ticketType, openTicket)
    UPER_ELEMENT_DEFAULT(KItinerary::Fcb::v13::LinkMode, linkMode, issuedTogether)
    UPER_GADGET_FINALIZE
};

/** Delay confirmation document. */
class DelayConfirmation {
    UPER_EXTENDABLE_GADGET
    UPER_ELEMENT_OPTIONAL(QByteArray, referenceIA5)
    UPER_ELEMENT_OPTIONAL(int, referenceNum)
    UPER_ELEMENT_OPTIONAL(int, trainNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, trainIA5)
    UPER_ELEMENT_OPTIONAL(int, departureYear)
    UPER_ELEMENT_OPTIONAL(int, departureDay)
    UPER_ELEMENT_OPTIONAL(int, departureTime)
    UPER_ELEMENT_OPTIONAL(int, departureUTCOffset)
    UPER_ELEMENT_DEFAULT(KItinerary::Fcb::v13::CodeTableType, stationCodeTable, stationUIC)
    UPER_ELEMENT_OPTIONAL(int, stationNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, stationIA5)
    UPER_ELEMENT(int, delay)
    UPER_ELEMENT(bool, trainCancelled)
    UPER_ELEMENT_DEFAULT(KItinerary::Fcb::v13::ConfirmationType, confirmationType, travelerDelayConfirmation)
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::v13::TicketLinkType>, affectedTickets)
    UPER_ELEMENT_OPTIONAL(QString, infoText)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::v13::ExtensionData, extension)
    UPER_GADGET_FINALIZE
};

/** Ticket token. */
class TokenType {
    UPER_GADGET
    UPER_ELEMENT_OPTIONAL(int, tokenProviderNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, tokenProviderIA5)
    UPER_ELEMENT_OPTIONAL(QByteArray, tokenSpecification)
    UPER_ELEMENT(QByteArray, token)
    UPER_GADGET_FINALIZE
};

/** Variant wrapper for specific document types.
 *  Can be one of:
 *  - ReservationData
 *  - CarCarriageReservationData
 *  - OpenTicketData
 *  - PassData
 *  - VoucherData
 *  - CustomerCardData
 *  - CountermarkData
 *  - ParkingGroundData
 *  - FIPTicketData
 *  - StationPassageData
 *  - ExtensionData
 *  - DelayConfirmation
 */
class DocumentData {
    UPER_EXTENDABLE_GADGET
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::v13::TokenType, token)
    UPER_ELEMENT(QVariant, ticket)
    UPER_GADGET_FINALIZE
};

/** Ticket control data. */
class ControlData {
    UPER_EXTENDABLE_GADGET
    UPER_ELEMENT_OPTIONAL(QList <KItinerary::Fcb::v13::CardReferenceType>, identificationByCardReference)
    UPER_ELEMENT(bool, identificationByIdCard)
    UPER_ELEMENT(bool, identificationByPassportId)
    UPER_ELEMENT_OPTIONAL(int, identificationItem)
    UPER_ELEMENT(bool, passportValidationRequired)
    UPER_ELEMENT(bool, onlineValidationRequired)
    UPER_ELEMENT_OPTIONAL(int, randomDetailedValidationRequired)
    UPER_ELEMENT(bool, ageCheckRequired)
    UPER_ELEMENT(bool, reductionCardCheckRequired)
    UPER_ELEMENT_OPTIONAL(QString, infoText)
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::v13::TicketLinkType>, includedTickets)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::v13::ExtensionData, extension)
    UPER_GADGET_FINALIZE
};

/** Top-level type for the ERA FCB ticket structure. */
class KITINERARY_EXPORT UicRailTicketData {
    UPER_EXTENDABLE_GADGET
    UPER_ELEMENT(KItinerary::Fcb::v13::IssuingData, issuingDetail)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::v13::TravelerData, travelerDetail)
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::v13::DocumentData>, transportDocument)
    UPER_ELEMENT_OPTIONAL(KItinerary::Fcb::v13::ControlData, controlDetail)
    UPER_ELEMENT_OPTIONAL(QList<KItinerary::Fcb::v13::ExtensionData>, extension)
    UPER_GADGET_FINALIZE
public:
    UicRailTicketData();
    explicit UicRailTicketData(const Uic9183Block &block);
    explicit UicRailTicketData(const QByteArray &data);

    [[nodiscard]] bool isValid() const;

private:
    QVariant m_data;
};

}
}

}

#endif // KITINERARY_FCBTICKET_H
