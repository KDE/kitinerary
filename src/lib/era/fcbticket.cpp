/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "fcbticket.h"

#include "asn1/uperdecoder.h"

#include <QDebug>

#define FCB_READ_CONSTRAINED_INT(Name, Min, Max) \
    if (Name ## IsSet()) \
        Name = decoder.readConstrainedWholeNumber(Min, Max)

#define FCB_READ_UNCONSTRAINED_INT(Name) \
    if (Name ## IsSet()) \
        Name = decoder.readUnconstrainedWholeNumber()

#define FCB_READ_IA5STRING(Name) \
    if (Name ## IsSet()) \
        Name = decoder.readIA5String()

#define FCB_READ_UTF8STRING(Name) \
    if (Name ## IsSet()) \
        Name = decoder.readUtf8String()

#define FCB_READ_ENUM(Name) \
    do { if (Name ## IsSet()) { \
        if constexpr (uperHasExtensionMarker(decltype(Name){})) { \
            Name = decoder.readEnumeratedWithExtensionMarker<decltype(Name)>(); \
        } else { \
            Name = decoder.readEnumerated<decltype(Name)>(); \
        } \
    } } while(false)

#define FCB_READ_CUSTOM(Name) \
    if (Name ## IsSet()) \
        Name.decode(decoder);

#define FCB_READ_SEQUENCE_OF_CONTRAINED_INT(Name, Min, Max) \
    if (Name ## IsSet()) \
        Name = decoder.readSequenceOfConstrainedWholeNumber(Min, Max)

#define FCB_READ_SEQUENCE_OF_CUSTOM(Name) \
    if (Name ## IsSet()) \
        Name = decoder.readSequenceOf<decltype(Name)::value_type>();

#define FCB_READ_INT_IA5_PAIR(Name, Min, Max) \
    FCB_READ_CONSTRAINED_INT(Name ## Num, Min, Max); \
    FCB_READ_IA5STRING(Name ## IA5)

#define FCB_READ_INT_IA5_PAIR_UNCONSTRAINED(Name) \
    FCB_READ_UNCONSTRAINED_INT(Name ## Num); \
    FCB_READ_IA5STRING(Name ## IA5)

using namespace KItinerary;

void Fcb::ExtensionData::decode(KItinerary::UPERDecoder &decoder)
{
    decodeSequence(decoder);
    extensionId = decoder.readIA5String();
    // extensionData =  TODO
}

void Fcb::GeoCoordinateType::decode(UPERDecoder &decoder)
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

void Fcb::IssuingData::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_INT_IA5_PAIR(securityProvider, 1, 32000);
    FCB_READ_INT_IA5_PAIR(issuer, 1, 32000);
    issuingYear = decoder.readConstrainedWholeNumber(2016, 2269);
    issuingDay = decoder.readConstrainedWholeNumber(1, 366);
    FCB_READ_CONSTRAINED_INT(issuingTime, 0, 1440);
    FCB_READ_UTF8STRING(issuerName);
    specimen = decoder.readBoolean();
    securePaperTicket = decoder.readBoolean();
    activated = decoder.readBoolean();
    if (currencyIsSet()) {
        currency = decoder.readIA5String(3, 3);
    }
    FCB_READ_CONSTRAINED_INT(currencyFract, 1, 3);
    FCB_READ_IA5STRING(issuerPNR);
    FCB_READ_CUSTOM(extension);
    FCB_READ_INT_IA5_PAIR_UNCONSTRAINED(issuedOnTrain);
    FCB_READ_UNCONSTRAINED_INT(issuedOnLine);
    FCB_READ_CUSTOM(pointOfSale);
}

void Fcb::CustomerStatusType::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_INT_IA5_PAIR_UNCONSTRAINED(statusProvider);
    FCB_READ_UNCONSTRAINED_INT(customerStatus);
    FCB_READ_IA5STRING(customerStatusDescr);
}

void Fcb::TravelerType::decode(UPERDecoder &decoder)
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
    FCB_READ_CONSTRAINED_INT(dayOfBirth, 0, 370);
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

void Fcb::TravelerData::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_SEQUENCE_OF_CUSTOM(traveler);
    if (preferredLanguageIsSet()) {
        preferredLanguage = decoder.readIA5String(2, 2);
    }
    FCB_READ_UTF8STRING(groupName);
}

void Fcb::TrainLinkType::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_INT_IA5_PAIR_UNCONSTRAINED(train);
    travelDate = decoder.readConstrainedWholeNumber(-1, 370);
    departureTime = decoder.readConstrainedWholeNumber(0, 1440);
    FCB_READ_CONSTRAINED_INT(departureUTCOffset, -60, 60);
    FCB_READ_INT_IA5_PAIR(fromStation, 1, 9999999);
    FCB_READ_INT_IA5_PAIR(toStation, 1, 9999999);
    FCB_READ_UTF8STRING(fromStationNameUTF8);
    FCB_READ_UTF8STRING(toStationNameUTF8);
}

void Fcb::RegionalValidityType::decode(UPERDecoder &decoder)
{
    assert(!decoder.readBoolean()); // TODO extension marker
    const auto choice = decoder.readConstrainedWholeNumber(0, 4);
    switch (choice) {
        case 0:
        {
            TrainLinkType v;
            v.decode(decoder);
            value = QVariant::fromValue(v);
            break;
        }
        default:
            qDebug() << choice; // TODO
            assert(false);
    }
}

void Fcb::ReturnRouteDescriptionType::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_INT_IA5_PAIR(fromStation, 1, 9999999);
    FCB_READ_INT_IA5_PAIR(toStation, 1, 9999999);
    FCB_READ_UTF8STRING(fromStationNameUTF8);
    FCB_READ_UTF8STRING(toStationNameUTF8);
    FCB_READ_UTF8STRING(validReturnRegionDesc);
    FCB_READ_SEQUENCE_OF_CUSTOM(validReturnRegion);
}

void Fcb::RouteSectionType::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_ENUM(stationCodeTable);
    FCB_READ_INT_IA5_PAIR(fromStation, 1, 9999999);
    FCB_READ_INT_IA5_PAIR(toStation, 1, 9999999);
    FCB_READ_UTF8STRING(fromStationNameUTF8);
    FCB_READ_UTF8STRING(toStationNameUTF8);
}

void Fcb::SeriesDetailType::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_CONSTRAINED_INT(supplyingCarrier, 1, 32000);
    FCB_READ_CONSTRAINED_INT(offerIdentification, 1, 99);
    FCB_READ_UNCONSTRAINED_INT(series);
}

void Fcb::CardReferenceType::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_INT_IA5_PAIR(cardIssuer, 1, 32000);
    FCB_READ_INT_IA5_PAIR_UNCONSTRAINED(cardId);
    FCB_READ_UTF8STRING(cardName);
    FCB_READ_UNCONSTRAINED_INT(cardType);
    FCB_READ_INT_IA5_PAIR_UNCONSTRAINED(leadingCardId);
    FCB_READ_INT_IA5_PAIR_UNCONSTRAINED(trailingCardId);
}

void Fcb::TariffType::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_CONSTRAINED_INT(numberOfPassengers, 1, 200);
    FCB_READ_ENUM(passengerType);
    FCB_READ_CONSTRAINED_INT(ageBelow, 1, 64);
    FCB_READ_CONSTRAINED_INT(ageAbove, 1, 128);
    FCB_READ_SEQUENCE_OF_CONTRAINED_INT(travelerid, 0, 254);
    restrictedToCountryOfResidence = decoder.readBoolean();
    FCB_READ_CUSTOM(restrictedToRouteSection);
    FCB_READ_CUSTOM(seriesDataDetails);
    FCB_READ_INT_IA5_PAIR_UNCONSTRAINED(tariffId);
    FCB_READ_UTF8STRING(tariffDesc);
    FCB_READ_SEQUENCE_OF_CUSTOM(reductionCard);
}

void Fcb::VatDetailType::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    country = decoder.readConstrainedWholeNumber(1, 999);
    percentage = decoder.readConstrainedWholeNumber(0, 999);
    FCB_READ_UNCONSTRAINED_INT(amount);
    FCB_READ_IA5STRING(vatId);
}

void Fcb::IncludedOpenTicketType::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_INT_IA5_PAIR(productOwner, 1, 32000);
    FCB_READ_INT_IA5_PAIR(productId, 0, 32000);
    FCB_READ_UNCONSTRAINED_INT(externalIssuerId);
    FCB_READ_UNCONSTRAINED_INT(issuerAutorizationId);
    FCB_READ_ENUM(stationCodeTable);
    FCB_READ_SEQUENCE_OF_CUSTOM(validRegion);
    FCB_READ_CONSTRAINED_INT(validFromDay, -1, 700);
    FCB_READ_CONSTRAINED_INT(validFromTime, 0, 1440);
    FCB_READ_CONSTRAINED_INT(validFromUTCOffset, -60, 60);
    FCB_READ_CONSTRAINED_INT(validUntilDay, 0, 370);
    FCB_READ_CONSTRAINED_INT(validUntilTime, 0, 1440);
    FCB_READ_CONSTRAINED_INT(validUntilUTCOffset, -60, 60);
    FCB_READ_ENUM(classCode);
    // TODO serviceLevel
    FCB_READ_SEQUENCE_OF_CONTRAINED_INT(carrierNum, 1, 32000);
    // TODO carrierIA5
    FCB_READ_SEQUENCE_OF_CONTRAINED_INT(includedServiceBrands, 1, 32000);
    FCB_READ_SEQUENCE_OF_CONTRAINED_INT(excludedServiceBrands, 1, 32000);
    FCB_READ_SEQUENCE_OF_CUSTOM(tariffs);
    FCB_READ_UTF8STRING(infoText);
    FCB_READ_CUSTOM(extension);
}

void Fcb::RegisteredLuggageType::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_IA5STRING(registrationId);
    FCB_READ_CONSTRAINED_INT(maxWeight, 1, 99);
    FCB_READ_CONSTRAINED_INT(maxSize, 1, 300);
}

void Fcb::LuggageRestrictionType::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_CONSTRAINED_INT(maxHandLuggagePieces, 0, 99);
    FCB_READ_CONSTRAINED_INT(maxNonHandLuggagePieces, 0, 99);
    FCB_READ_SEQUENCE_OF_CUSTOM(registeredLuggage);
}

void Fcb::OpenTicketData::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_INT_IA5_PAIR_UNCONSTRAINED(reference);
    FCB_READ_INT_IA5_PAIR(productOwner, 1, 32000);
    FCB_READ_INT_IA5_PAIR(productId, 0, 32000);
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
    FCB_READ_CONSTRAINED_INT(validFromDay, -1, 700);
    FCB_READ_CONSTRAINED_INT(validFromTime, 0, 1440);
    FCB_READ_CONSTRAINED_INT(validFromUTCOffset, -60, 60);
    FCB_READ_CONSTRAINED_INT(validUntilDay, 0, 370);
    FCB_READ_CONSTRAINED_INT(validUntilTime, 0, 1440);
    FCB_READ_CONSTRAINED_INT(validUntilUTCOffset, -60, 60);
    FCB_READ_SEQUENCE_OF_CONTRAINED_INT(activatedDay, 0, 370);
    FCB_READ_ENUM(classCode);
    FCB_READ_IA5STRING(serviceLevel); // TODO this is size constrained!
    FCB_READ_SEQUENCE_OF_CONTRAINED_INT(carrierNum, 1, 32000);
    // TODO carrierIA5
    FCB_READ_SEQUENCE_OF_CONTRAINED_INT(includedServiceBrands, 1, 32000);
    FCB_READ_SEQUENCE_OF_CONTRAINED_INT(excludedServiceBrands, 1, 32000);
    FCB_READ_SEQUENCE_OF_CUSTOM(tariffs);
    FCB_READ_UNCONSTRAINED_INT(price);
    FCB_READ_SEQUENCE_OF_CUSTOM(vatDetail);
    FCB_READ_UTF8STRING(infoText);
    FCB_READ_SEQUENCE_OF_CUSTOM(includedAddOns);
    FCB_READ_CUSTOM(luggage);
    FCB_READ_CUSTOM(extension);
}

void Fcb::TimeRangeType::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    fromTime = decoder.readConstrainedWholeNumber(0, 1440);
    untilTime = decoder.readConstrainedWholeNumber(0, 1440);
}

void Fcb::ValidityPeriodType::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_CONSTRAINED_INT(validFromDay, -1, 700);
    FCB_READ_CONSTRAINED_INT(validFromTime, 0, 1440);
    FCB_READ_CONSTRAINED_INT(validFromUTCOffset, -60, 60);
    FCB_READ_CONSTRAINED_INT(validUntilDay, 0, 370);
    FCB_READ_CONSTRAINED_INT(validUntilTime, 0, 1440);
    FCB_READ_CONSTRAINED_INT(validUntilUTCOffset, -60, 60);
}

void Fcb::ValidityPeriodDetailType::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_SEQUENCE_OF_CUSTOM(validityPeriod);
    FCB_READ_SEQUENCE_OF_CUSTOM(excludedTimeRange);
}

void Fcb::PassData::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_INT_IA5_PAIR_UNCONSTRAINED(reference);
    FCB_READ_INT_IA5_PAIR(productOwner, 1, 32000);
    FCB_READ_INT_IA5_PAIR(productId, 0, 32000);
    FCB_READ_CONSTRAINED_INT(passType, 1, 250);
    FCB_READ_UTF8STRING(passDescription);
    FCB_READ_ENUM(classCode);
    FCB_READ_CONSTRAINED_INT(validFromDay, -1, 700);
    FCB_READ_CONSTRAINED_INT(validFromTime, 0, 1440);
    FCB_READ_CONSTRAINED_INT(validFromUTCOffset, -60, 60);
    FCB_READ_CONSTRAINED_INT(validUntilDay, 0, 370);
    FCB_READ_CONSTRAINED_INT(validUntilTime, 0, 1440);
    FCB_READ_CONSTRAINED_INT(validUntilUTCOffset, -60, 60);
    FCB_READ_CUSTOM(validityPeriodDetails);
    FCB_READ_CONSTRAINED_INT(numberOfValidityDays, 0, 370);
    FCB_READ_CONSTRAINED_INT(numberOfPossibleTrips, 1, 250);
    FCB_READ_CONSTRAINED_INT(numberOfDaysOfTravel, 1, 250);
    FCB_READ_SEQUENCE_OF_CONTRAINED_INT(activatedDay, 0, 370);
    FCB_READ_SEQUENCE_OF_CONTRAINED_INT(countries, 1, 250);
    FCB_READ_SEQUENCE_OF_CONTRAINED_INT(includedCarrierNum, 1, 32000);
    // TODO includedCarrierIA5		SEQUENCE OF IA5String			OPTIONAL,
    FCB_READ_SEQUENCE_OF_CONTRAINED_INT(excludedCarrierNum, 1, 32000);
    // TODO excludedCarrierIA5		SEQUENCE OF IA5String			OPTIONAL,
    FCB_READ_SEQUENCE_OF_CONTRAINED_INT(includedServiceBrands, 1, 32000);
    FCB_READ_SEQUENCE_OF_CONTRAINED_INT(excludedServiceBrands, 1, 32000);
    FCB_READ_SEQUENCE_OF_CUSTOM(validRegion);
    FCB_READ_SEQUENCE_OF_CUSTOM(tariffs);
    FCB_READ_UNCONSTRAINED_INT(price);
    FCB_READ_SEQUENCE_OF_CUSTOM(vatDetail);
    FCB_READ_UTF8STRING(infoText);
    FCB_READ_CUSTOM(extension);
}

void Fcb::DocumentData::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    assert(!m_optionals[0]); // TODO token

    assert(!decoder.readBoolean()); // TODO extension marker for CHOICE
    const auto choiceIdx = decoder.readConstrainedWholeNumber(0, 11);
    switch (choiceIdx) {
        // TODO
        case 2: {
            OpenTicketData t;
            t.decode(decoder);
            ticket = QVariant::fromValue(t);
            break;
        }
        case 3: {
            PassData p;
            p.decode(decoder);
            ticket = QVariant::fromValue(p);
            break;
        }
    }
}

void Fcb::ControlData::decode(UPERDecoder &decoder)
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
    //FCB_READ_SEQUENCE_OF_CUSTOM(includedTickets); TODO
    FCB_READ_CUSTOM(extension);
}

Fcb::UicRailTicketData::UicRailTicketData() = default;

Fcb::UicRailTicketData::UicRailTicketData(const Uic9183Block &block)
    : m_block(block)
{
    UPERDecoder decoder(BitVectorView(std::string_view(block.content(), block.contentSize())));
    decode(decoder);
}

void Fcb::UicRailTicketData::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    issuingDetail.decode(decoder);
    FCB_READ_CUSTOM(travelerDetail);
    FCB_READ_SEQUENCE_OF_CUSTOM(transportDocument)
    FCB_READ_CUSTOM(controlDetail)
}

#include "moc_fcbticket.cpp"
