/*
    SPDX-FileCopyrightText: 2022-2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "fcbticket2.h"
#include "fcbreader_p.h"
#include "fcbutil.h"

#include "logging.h"
#include "asn1/uperdecoder.h"

constexpr inline auto FCB_TIME_MAX = 1439;
constexpr inline auto FCB_BEGIN_DATE_MIN = -1;
constexpr inline auto FCB_END_DATE_MIN = -1;
constexpr inline auto FCB_DATE_MAX = 370;

constexpr inline auto FCB_PRODUCT_ID_NUM_MAX = 65535;

using namespace KItinerary;

void Fcb::v2::ExtensionData::decode(KItinerary::UPERDecoder &decoder)
{
    decodeSequence(decoder);
    extensionId = decoder.readIA5String();
    extensionData = decoder.readOctetString();
}

void Fcb::v2::GeoCoordinateType::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_ENUM(geoUnit);
    FCB_READ_ENUM(coordinateSystem);
    FCB_READ_ENUM(hemisphereLongitude);
    FCB_READ_ENUM(hemisphereLongitude);
    longitude = decoder.readUnconstrainedWholeNumber();
    latitude = decoder.readUnconstrainedWholeNumber();
    FCB_READ_ENUM(accuracy);
}

void Fcb::v2::DeltaCoordinate::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    longitude = decoder.readUnconstrainedWholeNumber();
    latitude = decoder.readUnconstrainedWholeNumber();
}

void Fcb::v2::IssuingData::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_INT_IA5_PAIR(securityProvider, 1, 32000);
    FCB_READ_INT_IA5_PAIR(issuer, 1, 32000);
    issuingYear = decoder.readConstrainedWholeNumber(2016, 2269);
    issuingDay = decoder.readConstrainedWholeNumber(1, 366);
    FCB_READ_TIME(issuingTime);
    FCB_READ_UTF8STRING(issuerName);
    specimen = decoder.readBoolean();
    securePaperTicket = decoder.readBoolean();
    activated = decoder.readBoolean();
    FCB_READ_IA5STRING_CONSTRAINED(currency, 3, 3);
    FCB_READ_CONSTRAINED_INT(currencyFract, 1, 3);
    FCB_READ_IA5STRING(issuerPNR);
    FCB_READ_CUSTOM(extension);
    FCB_READ_INT_IA5_PAIR_UNCONSTRAINED(issuedOnTrain);
    FCB_READ_UNCONSTRAINED_INT(issuedOnLine);
    FCB_READ_CUSTOM(pointOfSale);
}

QDateTime Fcb::v2::IssuingData::issueingDateTime() const
{
    return FcbUtil::issuingDateTime(issuingYear, issuingDay, issuingTimeValue());
}

void Fcb::v2::CustomerStatusType::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_INT_IA5_PAIR_UNCONSTRAINED(statusProvider);
    FCB_READ_UNCONSTRAINED_INT(customerStatus);
    FCB_READ_IA5STRING(customerStatusDescr);
}

void Fcb::v2::TravelerType::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_UTF8STRING(firstName);
    FCB_READ_UTF8STRING(secondName);
    FCB_READ_UTF8STRING(lastName);
    FCB_READ_IA5STRING(idCard);
    FCB_READ_IA5STRING(passportId);
    FCB_READ_IA5STRING(title);
    FCB_READ_ENUM(gender);
    FCB_READ_IA5STRING(customerIdIA5);
    FCB_READ_UNCONSTRAINED_INT(customerIdNum);
    FCB_READ_CONSTRAINED_INT(yearOfBirth, 1901, 2155);
    FCB_READ_CONSTRAINED_INT(monthOfBirth, 1, 12);
    FCB_READ_CONSTRAINED_INT(dayOfBirthInMonth, 1, 31);
    ticketHolder = decoder.readBoolean();
    FCB_READ_ENUM(passengerType);
    if (passengerWithReducedMobilityIsSet()) {
        passengerWithReducedMobility = decoder.readBoolean();
    }
    FCB_READ_CONSTRAINED_INT(countryOfResidence, 1, 999);
    FCB_READ_CONSTRAINED_INT(countryOfPassport, 1, 999);
    FCB_READ_CONSTRAINED_INT(countryOfIdCard, 1, 999);
    FCB_READ_SEQUENCE_OF_CUSTOM(status);
}

void Fcb::v2::TravelerData::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_SEQUENCE_OF_CUSTOM(traveler);
    FCB_READ_IA5STRING_CONSTRAINED(preferredLanguage, 2, 2);
    FCB_READ_UTF8STRING(groupName);
}

void Fcb::v2::TrainLinkType::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_INT_IA5_PAIR_UNCONSTRAINED(train);
    travelDate = decoder.readConstrainedWholeNumber(-1, FCB_DATE_MAX);
    FCB_READ_TIME(departureTime);
    FCB_READ_CONSTRAINED_INT(departureUTCOffset, -60, 60);
    FCB_READ_INT_IA5_PAIR(fromStation, 1, 9999999);
    FCB_READ_INT_IA5_PAIR(toStation, 1, 9999999);
    FCB_READ_UTF8STRING(fromStationNameUTF8);
    FCB_READ_UTF8STRING(toStationNameUTF8);
}

QDateTime Fcb::v2::TrainLinkType::departureDateTime(const QDateTime &issueingDateTime) const
{
    return FcbUtil::decodeDifferentialTime(issueingDateTime, travelDate, departureTime, departureUTCOffsetValue());
}

void Fcb::v2::ViaStationType::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_ENUM(stationCodeTable);
    FCB_READ_INT_IA5_PAIR(station, 1, 9999999);
    FCB_READ_SEQUENCE_OF_CUSTOM(alternativeRoutes);
    FCB_READ_SEQUENCE_OF_CUSTOM(route);
    border = decoder.readBoolean();
    FCB_READ_SEQUENCE_OF_CONTRAINED_INT(carrierNum, 1, 32000);
    FCB_READ_SEQUENCE_OF_IA5STRING(carrierIA5);
    FCB_READ_UNCONSTRAINED_INT(seriesId);
    FCB_READ_UNCONSTRAINED_INT(routeId);
}

void Fcb::v2::ZoneType::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_INT_IA5_PAIR(carrier, 1, 32000);
    FCB_READ_ENUM(stationCodeTable);
    FCB_READ_INT_IA5_PAIR(entryStation, 1, 9999999);
    FCB_READ_INT_IA5_PAIR(terminatingStation, 1, 9999999);
    FCB_READ_UNCONSTRAINED_INT(city);
    FCB_READ_SEQUENCE_OF_UNCONTRAINED_INT(zoneId);
    FCB_READ_OCTETSTRING(binaryZoneId);
    FCB_READ_IA5STRING(nutsCode);
}

void Fcb::v2::LineType::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_INT_IA5_PAIR(carrier, 1, 32000);
    FCB_READ_SEQUENCE_OF_UNCONTRAINED_INT(lineId);
    FCB_READ_ENUM(stationCodeTable);
    FCB_READ_INT_IA5_PAIR(entryStation, 1, 9999999);
    FCB_READ_INT_IA5_PAIR(terminatingStation, 1, 9999999);
    FCB_READ_CONSTRAINED_INT(city, 1, 9999999);
}

void Fcb::v2::PolygoneType::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    firstEdge.decode(decoder);
    edges = decoder.readSequenceOf<DeltaCoordinate>();
}

void Fcb::v2::RegionalValidityType::decode(UPERDecoder &decoder)
{
    value = decoder.readChoiceWithExtensionMarker<decltype(value)>();
}

void Fcb::v2::ReturnRouteDescriptionType::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_INT_IA5_PAIR(fromStation, 1, 9999999);
    FCB_READ_INT_IA5_PAIR(toStation, 1, 9999999);
    FCB_READ_UTF8STRING(fromStationNameUTF8);
    FCB_READ_UTF8STRING(toStationNameUTF8);
    FCB_READ_UTF8STRING(validReturnRegionDesc);
    FCB_READ_SEQUENCE_OF_CUSTOM(validReturnRegion);
}

void Fcb::v2::RouteSectionType::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_ENUM(stationCodeTable);
    FCB_READ_INT_IA5_PAIR(fromStation, 1, 9999999);
    FCB_READ_INT_IA5_PAIR(toStation, 1, 9999999);
    FCB_READ_UTF8STRING(fromStationNameUTF8);
    FCB_READ_UTF8STRING(toStationNameUTF8);
}

void Fcb::v2::SeriesDetailType::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_CONSTRAINED_INT(supplyingCarrier, 1, 32000);
    FCB_READ_CONSTRAINED_INT(offerIdentification, 1, 99);
    FCB_READ_UNCONSTRAINED_INT(series);
}

void Fcb::v2::CardReferenceType::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_INT_IA5_PAIR(cardIssuer, 1, 32000);
    FCB_READ_INT_IA5_PAIR_UNCONSTRAINED(cardId);
    FCB_READ_UTF8STRING(cardName);
    FCB_READ_UNCONSTRAINED_INT(cardType);
    FCB_READ_INT_IA5_PAIR_UNCONSTRAINED(leadingCardId);
    FCB_READ_INT_IA5_PAIR_UNCONSTRAINED(trailingCardId);
}

void Fcb::v2::PlacesType::decode(KItinerary::UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_IA5STRING(coach);
    FCB_READ_IA5STRING(placeString);
    FCB_READ_UTF8STRING(placeDescription);
    FCB_READ_SEQUENCE_OF_IA5STRING(placeIA5);
    FCB_READ_SEQUENCE_OF_CONTRAINED_INT(placeNum, 1, 254);
}

void Fcb::v2::CompartmentDetailsType::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_CONSTRAINED_INT(coachType, 1, 99);
    FCB_READ_CONSTRAINED_INT(compartmentType, 1, 99);
    FCB_READ_CONSTRAINED_INT(specialAllocation, 1, 99);
    FCB_READ_UTF8STRING(coachTypeDescr);
    FCB_READ_UTF8STRING(compartmentTypeDescr);
    FCB_READ_UTF8STRING(specialAllocationDescr);
    FCB_READ_ENUM(position);
}

void Fcb::v2::BerthDetailData::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    berthType = decoder.readEnumerated<BerthTypeType>();
    numberOfBerths = decoder.readConstrainedWholeNumber(1, 999);
    FCB_READ_ENUM(gender);
}

void Fcb::v2::TariffType::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_CONSTRAINED_INT(numberOfPassengers, 1, 200);
    FCB_READ_ENUM(passengerType);
    FCB_READ_CONSTRAINED_INT(ageBelow, 1, 64);
    FCB_READ_CONSTRAINED_INT(ageAbove, 1, 128);
    FCB_READ_SEQUENCE_OF_CONTRAINED_INT(travelerid, 1, 254);
    restrictedToCountryOfResidence = decoder.readBoolean();
    FCB_READ_CUSTOM(restrictedToRouteSection);
    FCB_READ_CUSTOM(seriesDataDetails);
    FCB_READ_INT_IA5_PAIR_UNCONSTRAINED(tariffId);
    FCB_READ_UTF8STRING(tariffDesc);
    FCB_READ_SEQUENCE_OF_CUSTOM(reductionCard);
}

void Fcb::v2::VatDetailType::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    country = decoder.readConstrainedWholeNumber(1, 999);
    percentage = decoder.readConstrainedWholeNumber(0, 999);
    FCB_READ_UNCONSTRAINED_INT(amount);
    FCB_READ_IA5STRING(vatId);
}

void Fcb::v2::IncludedOpenTicketType::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_INT_IA5_PAIR(productOwner, 1, 32000);
    FCB_READ_INT_IA5_PAIR(productId, 0, FCB_PRODUCT_ID_NUM_MAX);
    FCB_READ_UNCONSTRAINED_INT(externalIssuerId);
    FCB_READ_UNCONSTRAINED_INT(issuerAutorizationId);
    FCB_READ_ENUM(stationCodeTable);
    FCB_READ_SEQUENCE_OF_CUSTOM(validRegion);
    FCB_READ_CONSTRAINED_INT(validFromDay, FCB_BEGIN_DATE_MIN, 700);
    FCB_READ_TIME(validFromTime);
    FCB_READ_CONSTRAINED_INT(validFromUTCOffset, -60, 60);
    FCB_READ_CONSTRAINED_INT(validUntilDay, FCB_END_DATE_MIN, FCB_DATE_MAX);
    FCB_READ_TIME(validUntilTime);
    FCB_READ_CONSTRAINED_INT(validUntilUTCOffset, -60, 60);
    FCB_READ_ENUM(classCode);
    FCB_READ_IA5STRING_CONSTRAINED(serviceLevel, 1, 2);
    FCB_READ_SEQUENCE_OF_CONTRAINED_INT(carrierNum, 1, 32000);
    FCB_READ_SEQUENCE_OF_IA5STRING(carrierIA5);
    FCB_READ_SEQUENCE_OF_CONTRAINED_INT(includedServiceBrands, 1, 32000);
    FCB_READ_SEQUENCE_OF_CONTRAINED_INT(excludedServiceBrands, 1, 32000);
    FCB_READ_SEQUENCE_OF_CUSTOM(tariffs);
    FCB_READ_UTF8STRING(infoText);
    FCB_READ_SEQUENCE_OF_CONTRAINED_INT(includedTransportType, 0, 31);
    FCB_READ_SEQUENCE_OF_CONTRAINED_INT(excludedTransportType, 0, 31);
    FCB_READ_CUSTOM(extension);
}

void Fcb::v2::RegisteredLuggageType::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_IA5STRING(registrationId);
    FCB_READ_CONSTRAINED_INT(maxWeight, 1, 99);
    FCB_READ_CONSTRAINED_INT(maxSize, 1, 300);
}

void Fcb::v2::LuggageRestrictionType::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_CONSTRAINED_INT(maxHandLuggagePieces, 0, 99);
    FCB_READ_CONSTRAINED_INT(maxNonHandLuggagePieces, 0, 99);
    FCB_READ_SEQUENCE_OF_CUSTOM(registeredLuggage);
}

void Fcb::v2::ReservationData::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_INT_IA5_PAIR_UNCONSTRAINED(train);
    FCB_READ_CONSTRAINED_INT(departureDate, -1, FCB_DATE_MAX);
    FCB_READ_IA5STRING(referenceIA5);
    FCB_READ_UNCONSTRAINED_INT(referenceNum);
    FCB_READ_INT_IA5_PAIR(productOwner, 1, 32000);
    FCB_READ_INT_IA5_PAIR(productId, 0, FCB_PRODUCT_ID_NUM_MAX);
    FCB_READ_CONSTRAINED_INT(serviceBrand, 0, 32000);
    FCB_READ_UTF8STRING(serviceBrandAbrUTF8);
    FCB_READ_UTF8STRING(serviceBrandNameUTF8);
    FCB_READ_ENUM(service);
    FCB_READ_ENUM(stationCodeTable);
    FCB_READ_INT_IA5_PAIR(fromStation, 1, 9999999);
    FCB_READ_INT_IA5_PAIR(toStation, 1, 9999999);
    FCB_READ_UTF8STRING(fromStationNameUTF8);
    FCB_READ_UTF8STRING(toStationNameUTF8);
    FCB_READ_TIME(departureTime);
    FCB_READ_CONSTRAINED_INT(departureUTCOffset, -60, 60);
    FCB_READ_CONSTRAINED_INT(arrivalDate, FCB_END_DATE_MIN, 20);
    FCB_READ_TIME(arrivalTime);
    FCB_READ_CONSTRAINED_INT(arrivalUTCOffset, -60, 60);
    FCB_READ_SEQUENCE_OF_CONTRAINED_INT(carrierNum, 1, 32000);
    FCB_READ_SEQUENCE_OF_IA5STRING(carrierIA5);
    FCB_READ_ENUM(classCode);
    FCB_READ_IA5STRING_CONSTRAINED(serviceLevel, 1, 2);
    FCB_READ_CUSTOM(places);
    FCB_READ_CUSTOM(additionalPlaces);
    FCB_READ_CUSTOM(bicyclePlaces);
    FCB_READ_CUSTOM(compartmentDetails);
    FCB_READ_CONSTRAINED_INT(numberOfOverbooked, 0, 200);
    FCB_READ_SEQUENCE_OF_CUSTOM(berth);
    FCB_READ_SEQUENCE_OF_CUSTOM(tariffs);
    FCB_READ_ENUM(priceType);
    FCB_READ_UNCONSTRAINED_INT(price);
    FCB_READ_SEQUENCE_OF_CUSTOM(vatDetail);
    FCB_READ_CONSTRAINED_INT(typeOfSupplement, 0, 9);
    FCB_READ_CONSTRAINED_INT(numberOfSupplements, 0, 200);
    FCB_READ_CUSTOM(luggage);
    FCB_READ_UTF8STRING(infoText);
    FCB_READ_CUSTOM(extension);
}

QDateTime Fcb::v2::ReservationData::departureDateTime(const QDateTime &issueingDateTime) const
{
    return FcbUtil::decodeDifferentialTime(issueingDateTime, departureDate, departureTimeValue(), departureUTCOffsetValue());
}

QDateTime Fcb::v2::ReservationData::arrivalDateTime(const QDateTime &issueingDateTime) const
{
    return FcbUtil::decodeDifferentialTime(departureDateTime(issueingDateTime), arrivalDate, arrivalTimeValue(), arrivalUTCOffsetValue());
}

void Fcb::v2::CarCarriageReservationData::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_INT_IA5_PAIR_UNCONSTRAINED(train);
    FCB_READ_CONSTRAINED_INT(beginLoadingDate, -1, FCB_DATE_MAX);
    FCB_READ_TIME(beginLoadingTime);
    FCB_READ_TIME(endLoadingTime);
    FCB_READ_CONSTRAINED_INT(loadingUTCOffset, -60, 60);
    FCB_READ_IA5STRING(referenceIA5);
    FCB_READ_UNCONSTRAINED_INT(referenceNum);
    FCB_READ_INT_IA5_PAIR(productOwner, 1, 32000);
    FCB_READ_INT_IA5_PAIR(productId, 0, FCB_PRODUCT_ID_NUM_MAX);
    FCB_READ_CONSTRAINED_INT(serviceBrand, 0, 32000);
    FCB_READ_UTF8STRING(serviceBrandAbrUTF8);
    FCB_READ_UTF8STRING(serviceBrandNameUTF8);
    FCB_READ_ENUM(stationCodeTable);
    FCB_READ_INT_IA5_PAIR(fromStation, 1, 9999999);
    FCB_READ_INT_IA5_PAIR(toStation, 1, 9999999);
    FCB_READ_UTF8STRING(fromStationNameUTF8);
    FCB_READ_UTF8STRING(toStationNameUTF8);
    FCB_READ_IA5STRING(coach);
    FCB_READ_IA5STRING(place);
    FCB_READ_CUSTOM(compartmentDetails);
    FCB_READ_IA5STRING(numberPlate);
    FCB_READ_IA5STRING(trailerPlate);
    carCategory = decoder.readConstrainedWholeNumber(0, 9);
    FCB_READ_CONSTRAINED_INT(boatCategory, 0, 9);
    textileRoof = decoder.readBoolean();
    FCB_READ_ENUM(roofRackType);
    FCB_READ_CONSTRAINED_INT(roofRackHeight, 0, 99);
    FCB_READ_CONSTRAINED_INT(attachedBoats, 0, 2);
    FCB_READ_CONSTRAINED_INT(attachedBicycles, 0, 4);
    FCB_READ_CONSTRAINED_INT(attachedSurfboards, 0, 5);
    FCB_READ_CONSTRAINED_INT(loadingListEntry, 0, 999);
    FCB_READ_ENUM(loadingDeck);
    FCB_READ_SEQUENCE_OF_CONTRAINED_INT(carrierNum, 1, 32000);
    FCB_READ_SEQUENCE_OF_IA5STRING(carrierIA5);
    tariff.decode(decoder);
    FCB_READ_ENUM(priceType);
    FCB_READ_UNCONSTRAINED_INT(price);
    FCB_READ_SEQUENCE_OF_CUSTOM(vatDetail);
    FCB_READ_UTF8STRING(infoText);
    FCB_READ_CUSTOM(extension);
}

void Fcb::v2::OpenTicketData::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_INT_IA5_PAIR_UNCONSTRAINED(reference);
    FCB_READ_INT_IA5_PAIR(productOwner, 1, 32000);
    FCB_READ_INT_IA5_PAIR(productId, 0, FCB_PRODUCT_ID_NUM_MAX);
    FCB_READ_UNCONSTRAINED_INT(extIssuerId);
    FCB_READ_UNCONSTRAINED_INT(issuerAutorizationId);
    returnIncluded = decoder.readBoolean();
    FCB_READ_ENUM(stationCodeTable);
    FCB_READ_INT_IA5_PAIR(fromStation, 1, 9999999);
    FCB_READ_INT_IA5_PAIR(toStation, 1, 9999999);
    FCB_READ_UTF8STRING(fromStationNameUTF8);
    FCB_READ_UTF8STRING(toStationNameUTF8);
    FCB_READ_UTF8STRING(validRegionDesc);
    FCB_READ_SEQUENCE_OF_CUSTOM(validRegion);
    FCB_READ_CUSTOM(returnDescription);
    FCB_READ_CONSTRAINED_INT(validFromDay, FCB_BEGIN_DATE_MIN, 700);
    FCB_READ_TIME(validFromTime);
    FCB_READ_CONSTRAINED_INT(validFromUTCOffset, -60, 60);
    FCB_READ_CONSTRAINED_INT(validUntilDay, FCB_END_DATE_MIN, FCB_DATE_MAX);
    FCB_READ_TIME(validUntilTime);
    FCB_READ_CONSTRAINED_INT(validUntilUTCOffset, -60, 60);
    FCB_READ_SEQUENCE_OF_CONTRAINED_INT(activatedDay, 0, FCB_DATE_MAX);
    FCB_READ_ENUM(classCode);
    FCB_READ_IA5STRING_CONSTRAINED(serviceLevel, 1, 2);
    FCB_READ_SEQUENCE_OF_CONTRAINED_INT(carrierNum, 1, 32000);
    FCB_READ_SEQUENCE_OF_IA5STRING(carrierIA5);
    FCB_READ_SEQUENCE_OF_CONTRAINED_INT(includedServiceBrands, 1, 32000);
    FCB_READ_SEQUENCE_OF_CONTRAINED_INT(excludedServiceBrands, 1, 32000);
    FCB_READ_SEQUENCE_OF_CUSTOM(tariffs);
    FCB_READ_UNCONSTRAINED_INT(price);
    FCB_READ_SEQUENCE_OF_CUSTOM(vatDetail);
    FCB_READ_UTF8STRING(infoText);
    FCB_READ_SEQUENCE_OF_CUSTOM(includedAddOns);
    FCB_READ_CUSTOM(luggage);
    FCB_READ_SEQUENCE_OF_CONTRAINED_INT(includedTransportType, 0, 31);
    FCB_READ_SEQUENCE_OF_CONTRAINED_INT(excludedTransportType, 0, 31);
    FCB_READ_CUSTOM(extension);
}

QDateTime Fcb::v2::OpenTicketData::validFrom(const QDateTime &issueingDateTime) const
{
    return FcbUtil::decodeDifferentialStartTime(issueingDateTime, validFromDay, validFromTimeValue(), validFromUTCOffsetValue());
}

QDateTime Fcb::v2::OpenTicketData::validUntil(const QDateTime &issueingDateTime) const
{
    return FcbUtil::decodeDifferentialEndTime(validFrom(issueingDateTime), validUntilDay, validUntilTimeValue(), validFromUTCOffsetValue());
}

void Fcb::v2::TimeRangeType::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_TIME(fromTime);
    FCB_READ_TIME(untilTime);
}

void Fcb::v2::ValidityPeriodType::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_CONSTRAINED_INT(validFromDay, FCB_BEGIN_DATE_MIN, 700);
    FCB_READ_TIME(validFromTime);
    FCB_READ_CONSTRAINED_INT(validFromUTCOffset, -60, 60);
    FCB_READ_CONSTRAINED_INT(validUntilDay, FCB_END_DATE_MIN, FCB_DATE_MAX);
    FCB_READ_TIME(validUntilTime);
    FCB_READ_CONSTRAINED_INT(validUntilUTCOffset, -60, 60);
}

void Fcb::v2::ValidityPeriodDetailType::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_SEQUENCE_OF_CUSTOM(validityPeriod);
    FCB_READ_SEQUENCE_OF_CUSTOM(excludedTimeRange);
}

void Fcb::v2::PassData::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_INT_IA5_PAIR_UNCONSTRAINED(reference);
    FCB_READ_INT_IA5_PAIR(productOwner, 1, 32000);
    FCB_READ_INT_IA5_PAIR(productId, 0, FCB_PRODUCT_ID_NUM_MAX);
    FCB_READ_CONSTRAINED_INT(passType, 1, 250);
    FCB_READ_UTF8STRING(passDescription);
    FCB_READ_ENUM(classCode);
    FCB_READ_CONSTRAINED_INT(validFromDay, FCB_BEGIN_DATE_MIN, 700);
    FCB_READ_TIME(validFromTime);
    FCB_READ_CONSTRAINED_INT(validFromUTCOffset, -60, 60);
    FCB_READ_CONSTRAINED_INT(validUntilDay, FCB_END_DATE_MIN, FCB_DATE_MAX);
    FCB_READ_TIME(validUntilTime);
    FCB_READ_CONSTRAINED_INT(validUntilUTCOffset, -60, 60);
    FCB_READ_CUSTOM(validityPeriodDetails);
    FCB_READ_CONSTRAINED_INT(numberOfValidityDays, 0, FCB_DATE_MAX);
    FCB_READ_CONSTRAINED_INT(numberOfPossibleTrips, 1, 250);
    FCB_READ_CONSTRAINED_INT(numberOfDaysOfTravel, 1, 250);
    FCB_READ_SEQUENCE_OF_CONTRAINED_INT(activatedDay, 0, FCB_DATE_MAX);
    FCB_READ_SEQUENCE_OF_CONTRAINED_INT(countries, 1, 250);
    FCB_READ_SEQUENCE_OF_CONTRAINED_INT(includedCarrierNum, 1, 32000);
    FCB_READ_SEQUENCE_OF_IA5STRING(includedCarrierIA5);
    FCB_READ_SEQUENCE_OF_CONTRAINED_INT(excludedCarrierNum, 1, 32000);
    FCB_READ_SEQUENCE_OF_IA5STRING(excludedCarrierIA5);
    FCB_READ_SEQUENCE_OF_CONTRAINED_INT(includedServiceBrands, 1, 32000);
    FCB_READ_SEQUENCE_OF_CONTRAINED_INT(excludedServiceBrands, 1, 32000);
    FCB_READ_SEQUENCE_OF_CUSTOM(validRegion);
    FCB_READ_SEQUENCE_OF_CUSTOM(tariffs);
    FCB_READ_UNCONSTRAINED_INT(price);
    FCB_READ_SEQUENCE_OF_CUSTOM(vatDetail);
    FCB_READ_UTF8STRING(infoText);
    FCB_READ_CUSTOM(extension);
}

QDateTime Fcb::v2::PassData::validFrom(const QDateTime &issueingDateTime) const
{
    return FcbUtil::decodeDifferentialStartTime(issueingDateTime, validFromDay, validFromTimeValue(), validFromUTCOffsetValue());
}

QDateTime Fcb::v2::PassData::validUntil(const QDateTime &issueingDateTime) const
{
    return FcbUtil::decodeDifferentialEndTime(validFrom(issueingDateTime), validUntilDay, validUntilTimeValue(), validUntilUTCOffsetValue());
}

void Fcb::v2::VoucherData::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_IA5STRING(referenceIA5);
    FCB_READ_UNCONSTRAINED_INT(referenceNum);
    FCB_READ_INT_IA5_PAIR(productOwner, 1, 32000);
    FCB_READ_INT_IA5_PAIR(productId, 0, FCB_PRODUCT_ID_NUM_MAX);
    validFromYear = decoder.readConstrainedWholeNumber(2016, 2269);
    validFromDay = decoder.readConstrainedWholeNumber(0, FCB_DATE_MAX);
    validUntilYear = decoder.readConstrainedWholeNumber(2016, 2269);
    validUntilDay = decoder.readConstrainedWholeNumber(0, FCB_DATE_MAX);
    FCB_READ_UNCONSTRAINED_INT(value);
    FCB_READ_CONSTRAINED_INT(type, 1, 32000);
    FCB_READ_UTF8STRING(infoText);
    FCB_READ_CUSTOM(extension);
}

void Fcb::v2::CustomerCardData::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_CUSTOM(customer);
    FCB_READ_IA5STRING(cardIdIA5);
    FCB_READ_UNCONSTRAINED_INT(cardIdNum);
    validFromYear = decoder.readConstrainedWholeNumber(2016, 2269);
    FCB_READ_CONSTRAINED_INT(validFromDay, 0, FCB_DATE_MAX);
    FCB_READ_CONSTRAINED_INT(validUntilYear, 0, 250);
    FCB_READ_CONSTRAINED_INT(validUntilDay, 0, FCB_DATE_MAX);
    FCB_READ_ENUM(classCode);
    FCB_READ_CONSTRAINED_INT(cardType, 1, 1000);
    FCB_READ_UTF8STRING(cardTypeDescr);
    FCB_READ_UNCONSTRAINED_INT(customerStatus);
    FCB_READ_IA5STRING(customerStatusDescr);
    FCB_READ_SEQUENCE_OF_UNCONTRAINED_INT(includedServices);
    FCB_READ_CUSTOM(extension);
}

QDate Fcb::v2::CustomerCardData::validFrom() const
{
    return FcbUtil::decodeDate(validFromYear, validFromDayValue());
}

QDate Fcb::v2::CustomerCardData::validUntil() const
{
    return FcbUtil::decodeDifferentialDate(validFrom(), validUntilYear, validFromDayValue());
}

void Fcb::v2::CountermarkData::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_IA5STRING(referenceIA5);
    FCB_READ_UNCONSTRAINED_INT(referenceNum);
    FCB_READ_INT_IA5_PAIR(productOwner, 1, 32000);
    FCB_READ_INT_IA5_PAIR(productId, 0, FCB_PRODUCT_ID_NUM_MAX);
    FCB_READ_IA5STRING(ticketReferenceIA5);
    FCB_READ_UNCONSTRAINED_INT(ticketReferenceNum);
    numberOfCountermark = decoder.readConstrainedWholeNumber(1, 200);
    totalOfCountermarks = decoder.readConstrainedWholeNumber(1, 200);
    groupName = decoder.readUtf8String();
    FCB_READ_ENUM(stationCodeTable);
    FCB_READ_INT_IA5_PAIR(fromStation, 1, 9999999);
    FCB_READ_INT_IA5_PAIR(toStation, 1, 9999999);
    FCB_READ_UTF8STRING(fromStationNameUTF8);
    FCB_READ_UTF8STRING(toStationNameUTF8);
    FCB_READ_UTF8STRING(validRegionDesc);
    FCB_READ_SEQUENCE_OF_CUSTOM(validRegion);
    returnIncluded = decoder.readBoolean();
    FCB_READ_CUSTOM(returnDescription);
    FCB_READ_CONSTRAINED_INT(validFromDay, FCB_BEGIN_DATE_MIN, 700);
    FCB_READ_TIME(validFromTime);
    FCB_READ_CONSTRAINED_INT(validFromUTCOffset, -60, 60);
    FCB_READ_CONSTRAINED_INT(validUntilDay, FCB_END_DATE_MIN, FCB_DATE_MAX);
    FCB_READ_TIME(validUntilTime);
    FCB_READ_CONSTRAINED_INT(validUntilUTCOffset, -60, 60);
    FCB_READ_ENUM(classCode);
    FCB_READ_SEQUENCE_OF_CONTRAINED_INT(carrierNum, 1, 32000);
    FCB_READ_SEQUENCE_OF_IA5STRING(carrierIA5);
    FCB_READ_SEQUENCE_OF_CONTRAINED_INT(includedServiceBrands, 1, 32000);
    FCB_READ_SEQUENCE_OF_CONTRAINED_INT(excludedServiceBrands, 1, 32000);
    FCB_READ_UTF8STRING(infoText);
    FCB_READ_CUSTOM(extension);
}

void Fcb::v2::ParkingGroundData::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_IA5STRING(referenceIA5);
    FCB_READ_UNCONSTRAINED_INT(referenceNum);
    parkingGroundId = decoder.readIA5String();
    fromParkingDate = decoder.readConstrainedWholeNumber(FCB_BEGIN_DATE_MIN, 370);
    FCB_READ_CONSTRAINED_INT(untilParkingDate, 0, FCB_DATE_MAX);
    FCB_READ_INT_IA5_PAIR(productOwner, 1, 32000);
    FCB_READ_INT_IA5_PAIR(productId, 0, FCB_PRODUCT_ID_NUM_MAX);
    FCB_READ_IA5STRING(accessCode);
    location = decoder.readUtf8String();
    FCB_READ_ENUM(stationCodeTable);
    FCB_READ_UNCONSTRAINED_INT(stationNum);
    FCB_READ_UTF8STRING(stationIA5);
    FCB_READ_UTF8STRING(specialInformation);
    FCB_READ_UTF8STRING(entryTrack);
    FCB_READ_IA5STRING(numberPlate);
    FCB_READ_UNCONSTRAINED_INT(price);
    FCB_READ_SEQUENCE_OF_CUSTOM(vatDetail);
    FCB_READ_CUSTOM(extension);
}

void Fcb::v2::FIPTicketData::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_IA5STRING(referenceIA5);
    FCB_READ_UNCONSTRAINED_INT(referenceNum);
    FCB_READ_INT_IA5_PAIR(productOwner, 1, 32000);
    FCB_READ_INT_IA5_PAIR(productId, 0, FCB_PRODUCT_ID_NUM_MAX);
    FCB_READ_CONSTRAINED_INT(validFromDay, -1, 700);
    FCB_READ_CONSTRAINED_INT(validUntilDay, 0, FCB_DATE_MAX);
    FCB_READ_SEQUENCE_OF_CONTRAINED_INT(activatedDay, 0, FCB_DATE_MAX);
    FCB_READ_SEQUENCE_OF_CONTRAINED_INT(carrierNum, 1, 32000);
    FCB_READ_SEQUENCE_OF_IA5STRING(carrierIA5);
    numberOfTravelDays = decoder.readConstrainedWholeNumber(1, 200);
    includesSupplements = decoder.readBoolean();
    FCB_READ_ENUM(classCode);
    FCB_READ_CUSTOM(extension);
}

void Fcb::v2::StationPassageData::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_IA5STRING(referenceIA5);
    FCB_READ_UNCONSTRAINED_INT(referenceNum);
    FCB_READ_INT_IA5_PAIR(productOwner, 1, 32000);
    FCB_READ_INT_IA5_PAIR(productId, 0, FCB_PRODUCT_ID_NUM_MAX);
    FCB_READ_UTF8STRING(productName);
    FCB_READ_ENUM(stationCodeTable);
    FCB_READ_SEQUENCE_OF_UNCONTRAINED_INT(stationNum);
    FCB_READ_SEQUENCE_OF_IA5STRING(stationIA5);
    FCB_READ_SEQUENCE_OF_UTF8STRING(stationNameUTF8);
    FCB_READ_SEQUENCE_OF_UNCONTRAINED_INT(areaCodeNum);
    FCB_READ_SEQUENCE_OF_IA5STRING(areaCodeIA5);
    FCB_READ_SEQUENCE_OF_UTF8STRING(areaNameUTF8);
    validFromDay = decoder.readConstrainedWholeNumber(FCB_BEGIN_DATE_MIN, 700);
    FCB_READ_TIME(validFromTime);
    FCB_READ_CONSTRAINED_INT(validFromUTCOffset, -60, 60);
    FCB_READ_CONSTRAINED_INT(validUntilDay, FCB_END_DATE_MIN, FCB_DATE_MAX);
    FCB_READ_TIME(validUntilTime);
    FCB_READ_CONSTRAINED_INT(validUntilUTCOffset, -60, 60);
    FCB_READ_UNCONSTRAINED_INT(numberOfDaysValid);
    FCB_READ_CUSTOM(extension);
}

void Fcb::v2::DelayConfirmation::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_IA5STRING(referenceIA5);
    FCB_READ_UNCONSTRAINED_INT(referenceNum);
    FCB_READ_INT_IA5_PAIR_UNCONSTRAINED(train);
    FCB_READ_CONSTRAINED_INT(departureYear, 2016, 2269);
    FCB_READ_CONSTRAINED_INT(departureDay, 1, 366);
    FCB_READ_TIME(departureTime);
    FCB_READ_CONSTRAINED_INT(departureUTCOffset, -60, 60);
    FCB_READ_ENUM(stationCodeTable);
    FCB_READ_INT_IA5_PAIR(station, 1, 9999999);
    delay = decoder.readConstrainedWholeNumber(1, 999);
    trainCancelled = decoder.readBoolean();
    FCB_READ_ENUM(confirmationType);
    FCB_READ_SEQUENCE_OF_CUSTOM(affectedTickets);
    FCB_READ_UTF8STRING(infoText);
    FCB_READ_CUSTOM(extension);
}

void Fcb::v2::TokenType::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_INT_IA5_PAIR_UNCONSTRAINED(tokenProvider);
    FCB_READ_IA5STRING(tokenSpecification);
    token = decoder.readOctetString();
}

void Fcb::v2::DocumentData::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_CUSTOM(token);
    ticket = decoder.readChoiceWithExtensionMarker<decltype(ticket)>();
}

void Fcb::v2::TicketLinkType::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_IA5STRING(referenceIA5);
    FCB_READ_UNCONSTRAINED_INT(referenceNum);
    FCB_READ_UTF8STRING(issuerName);
    FCB_READ_IA5STRING(issuerPNR);
    FCB_READ_INT_IA5_PAIR(productOwner, 1, 32000);
    FCB_READ_ENUM(ticketType);
    FCB_READ_ENUM(linkMode);
}

void Fcb::v2::ControlData::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_SEQUENCE_OF_CUSTOM(identificationByCardReference);
    identificationByIdCard = decoder.readBoolean();
    identificationByPassportId = decoder.readBoolean();
    FCB_READ_UNCONSTRAINED_INT(identificationItem);
    passportValidationRequired = decoder.readBoolean();
    onlineValidationRequired = decoder.readBoolean();
    FCB_READ_CONSTRAINED_INT(randomDetailedValidationRequired, 0, 99);
    ageCheckRequired = decoder.readBoolean();
    reductionCardCheckRequired = decoder.readBoolean();
    FCB_READ_UTF8STRING(infoText);
    FCB_READ_SEQUENCE_OF_CUSTOM(includedTickets);
    FCB_READ_CUSTOM(extension);
}

Fcb::v2::UicRailTicketData::UicRailTicketData() = default;

Fcb::v2::UicRailTicketData::UicRailTicketData(const Uic9183Block &block)
    : m_data(QVariant::fromValue(block))
{
    if (block.isNull()) {
        return;
    }
    UPERDecoder decoder(BitVectorView(std::string_view(block.content(), block.contentSize())));
    decode(decoder);
    if (decoder.hasError()) {
        qCWarning(Log) << decoder.errorMessage();
        m_data = {};
    }
}

Fcb::v2::UicRailTicketData::UicRailTicketData(const QByteArray &data)
    : m_data(data)
{
    UPERDecoder decoder(BitVectorView(std::string_view(data.constData(), data.size())));
    decode(decoder);
    if (decoder.hasError()) {
        qCWarning(Log) << decoder.errorMessage();
        m_data = {};
    }
}

void Fcb::v2::UicRailTicketData::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    issuingDetail.decode(decoder);
    FCB_READ_CUSTOM(travelerDetail);
    FCB_READ_SEQUENCE_OF_CUSTOM(transportDocument)
    FCB_READ_CUSTOM(controlDetail)
}

bool Fcb::v2::UicRailTicketData::isValid() const
{
    return !m_data.isNull();
}

#include "moc_fcbticket2.cpp"
