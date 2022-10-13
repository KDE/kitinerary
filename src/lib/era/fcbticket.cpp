/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "fcbticket.h"

#include "asn1/uperdecoder.h"

using namespace KItinerary;

void Fcb::IssuingData::decode(UPERDecoder &decoder)
{
    assert(!decoder.readBoolean()); // TODO extension marker
    m_optionals = decoder.readBitset<14>();
    if (securityProviderNumIsSet()) {
        securityProviderNum = decoder.readConstrainedWholeNumber(1, 32000);
    }
    if (securityProviderIA5IsSet()) {
        securityProviderIA5 = decoder.readIA5String();
    }
    if (issuerNumIsSet()) {
        issuerNum = decoder.readConstrainedWholeNumber(1, 32000);
    }
    if (issuerIA5IsSet()) {
        issuerIA5 = decoder.readIA5String();
    }
    issuingYear = decoder.readConstrainedWholeNumber(2016, 2269);
    issuingDay = decoder.readConstrainedWholeNumber(1, 366);
    if (issuingTimeIsSet()) {
        issuingTime = decoder.readConstrainedWholeNumber(0, 1440);
    }
    if (issuerNameIsSet()) {
        issuerName = decoder.readUtf8String();
    }
    specimen = decoder.readBoolean();
    securePaperTicket = decoder.readBoolean();
    activated = decoder.readBoolean();
    if (currencyIsSet()) {
        currency = decoder.readIA5String(3, 3);
    }
    if (currencyFractIsSet()) {
        currencyFract = decoder.readConstrainedWholeNumber(1, 3);
    }
    if (issuerPNRIsSet()) {
        issuerPNR = decoder.readIA5String();
    }
    assert(!m_optionals[4]); // TODO ExtensionData
    if (issuedOnTrainNumIsSet()) {
        issuedOnTrainNum = decoder.readUnconstrainedWholeNumber();
    }
    if (issuedOnTrainNumIsSet()) {
        issuedOnTrainIA5 = decoder.readIA5String();
    }
    if (issuedOnLineIsSet()) {
        issuedOnLine = decoder.readUnconstrainedWholeNumber();
    }
    assert(!m_optionals[0]); // TODO GeoCoordinateType
}

void Fcb::TravelerType::decode(UPERDecoder &decoder)
{
    assert(!decoder.readBoolean()); // TODO extension marker

    m_optionals = decoder.readBitset<17>();
    if (firstNameIsSet()) {
        firstName = decoder.readUtf8String();
    }
    if (secondNameIsSet()) {
        secondName = decoder.readUtf8String();
    }
    if (lastNameIsSet()) {
        lastName = decoder.readUtf8String();
    }
    if (idCardIsSet()) {
        idCard = decoder.readIA5String();
    }
    if (passportIdIsSet()) {
        passportId = decoder.readIA5String();
    }
    if (titleIsSet()) {
        title = decoder.readIA5String(1, 3);
    }
    if (genderIsSet()) {
        gender = decoder.readEnumeratedWithExtensionMarker<GenderType>();
    }
    if (customerIdIA5IsSet()) {
        customerIdIA5 = decoder.readIA5String();
    }
    if (customerIdNumIsSet()) {
        customerIdNum = decoder.readUnconstrainedWholeNumber();
    }
    if (yearOfBirthIsSet()) {
        yearOfBirth = decoder.readConstrainedWholeNumber(1901, 2155);
    }
    if (dayOfBirthIsSet()) {
        dayOfBirth = decoder.readConstrainedWholeNumber(0, 370);
    }
    ticketHolder = decoder.readBoolean();
    if (passengerTypeIsSet()) {
        passengerType = decoder.readEnumeratedWithExtensionMarker<PassengerType>();
    }
    if (passengerWithReducedMobilityIsSet()) {
        passengerWithReducedMobility = decoder.readBoolean();
    }
    if (countryOfResidenceIsSet()) {
        countryOfResidence = decoder.readConstrainedWholeNumber(1, 999);
    }
    if (countryOfPassportIsSet()) {
        countryOfPassport = decoder.readConstrainedWholeNumber(1, 999);
    }
    if (countryOfIdCardIsSet()) {
        countryOfIdCard = decoder.readConstrainedWholeNumber(1, 999);
    }
    assert(!m_optionals[0]); // TODO status
}

void Fcb::TravelerData::decode(UPERDecoder &decoder)
{
    assert(!decoder.readBoolean()); // TODO extension marker

    m_optionals = decoder.readBitset<3>();
    if (travelerIsSet()) {
        traveler = decoder.readSequenceOf<TravelerType>();
    }
    if (preferredLanguageIsSet()) {
        preferredLanguage = decoder.readIA5String(2, 2);
    }
    if (groupNameIsSet()) {
        groupName = decoder.readUtf8String();
    }
}

void Fcb::TrainLinkType::decode(UPERDecoder &decoder)
{
    m_optionals = decoder.readBitset<9>();
    if (trainNumIsSet()) {
        trainNum = decoder.readUnconstrainedWholeNumber();
    }
    if (trainIA5IsSet()) {
        trainIA5 = decoder.readIA5String();
    }
    travelDate = decoder.readConstrainedWholeNumber(-1, 370);
    departureTime = decoder.readConstrainedWholeNumber(0, 1440);
    if (departureUTCOffsetIsSet()) {
        departureUTCOffset = decoder.readConstrainedWholeNumber(-60, 60);
    }
    if (fromStationNumIsSet()) {
        fromStationNum = decoder.readConstrainedWholeNumber(1, 9999999);
    }
    if (fromStationIA5IsSet()) {
        fromStationIA5 = decoder.readIA5String();
    }
    if (toStationNumIsSet()) {
        toStationNum = decoder.readConstrainedWholeNumber(1, 9999999);
    }
    if (toStationIA5IsSet()) {
        toStationIA5 = decoder.readIA5String();
    }
    if (fromStationNameUTF8IsSet()) {
        fromStationNameUTF8 = decoder.readUtf8String();
    }
    if (toStationNameUTF8IsSet()) {
        toStationNameUTF8 = decoder.readUtf8String();
    }
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
    // TODO
    assert(false);
}

void Fcb::OpenTicketData::decode(UPERDecoder &decoder)
{
    assert(!decoder.readBoolean()); // TODO extension marker

    m_optionals = decoder.readBitset<38>();
    if (referenceNumIsSet()) {
        referenceNum = decoder.readUnconstrainedWholeNumber();
    }
    if (referenceIA5IsSet()) {
        referenceIA5 = decoder.readIA5String();
    }
    if (productOwnerNumIsSet()) {
        productOwnerNum = decoder.readConstrainedWholeNumber(1, 32000);
    }
    if (productOwnerIA5IsSet()) {
        productOwnerIA5 = decoder.readIA5String();
    }
    if (productIdNumIsSet()) {
        productIdNum = decoder.readConstrainedWholeNumber(0, 32000);
    }
    if (productIdIA5IsSet()) {
        productIdIA5 = decoder.readIA5String();
    }
    if (extIssuerIdIsSet()) {
        extIssuerId = decoder.readUnconstrainedWholeNumber();
    }
    if (issuerAutorizationIdIsSet()) {
        issuerAutorizationId = decoder.readUnconstrainedWholeNumber();
    }
    returnIncluded = decoder.readBoolean();
    if (stationCodeTableIsSet()) {
        stationCodeTable = decoder.readEnumerated<CodeTableType>();
    }
    if (fromStationNumIsSet()) {
        fromStationNum = decoder.readConstrainedWholeNumber(1, 9999999);
    }
    if (fromStationIA5IsSet()) {
        fromStationIA5 = decoder.readIA5String();
    }
    if (toStationNumIsSet()) {
        toStationNum = decoder.readConstrainedWholeNumber(1, 9999999);
    }
    if (toStationIA5IsSet()) {
        toStationIA5 = decoder.readIA5String();
    }
    if (fromStationNameUTF8IsSet()) {
        fromStationNameUTF8 = decoder.readUtf8String();
    }
    if (toStationNameUTF8IsSet()) {
        toStationNameUTF8 = decoder.readUtf8String();
    }
    if (validRegionDescIsSet()) {
        validRegionDesc = decoder.readUtf8String();
    }
    if (validRegionIsSet()) {
        validRegion = decoder.readSequenceOf<RegionalValidityType>();
    }
    if (returnDescriptionIsSet()) {
        returnDescription.decode(decoder);
    }
    if (validFromDayIsSet()) {
        validFromDay = decoder.readConstrainedWholeNumber(-1, 700);
    }
    if (validFromTimeIsSet()) {
        validFromTime = decoder.readConstrainedWholeNumber(0, 1440);
    }
    if (validFromUTCOffsetIsSet()) {
        validFromUTCOffset = decoder.readConstrainedWholeNumber(-60, 60);
    }
    if (validUntilDayIsSet()) {
        validUntilDay = decoder.readConstrainedWholeNumber(0, 370);
    }
    if (validUntilTimeIsSet()) {
        validUntilTime = decoder.readConstrainedWholeNumber(0, 1440);
    }
    if (validUntilUTCOffsetIsSet()) {
        validUntilUTCOffset = decoder.readConstrainedWholeNumber(-60, 60);
    }
    // TODO activatedDay
    if (classCodeIsSet()) {
        classCode = decoder.readEnumeratedWithExtensionMarker<TravelClassType>();
    }
    if (serviceLevelIsSet()) {
        serviceLevel = decoder.readIA5String(1, 2);
    }
    // TODO
}

void Fcb::DocumentData::decode(UPERDecoder &decoder)
{
    assert(!decoder.readBoolean()); // TODO extension marker

    m_optionals = decoder.readBitset<1>();
    assert(!m_optionals[0]); // TODO token

    assert(!decoder.readBoolean()); // TODO extension marker for CHOICE
    const auto choiceIdx = decoder.readConstrainedWholeNumber(0, 11);
    switch (choiceIdx) {
        // TODO
        case 2: {
            OpenTicketData t;
            t.decode(decoder);
            ticket = QVariant::fromValue(t);
        }
    }
}

Fcb::UicRailTicketData::UicRailTicketData() = default;

Fcb::UicRailTicketData::UicRailTicketData(const Uic9183Block &block)
    : m_block(block)
{
    UPERDecoder decoder(BitVector(QByteArray(block.content(), block.contentSize()))); // TODO make BitVector a view
    decode(decoder);
}

void Fcb::UicRailTicketData::decode(UPERDecoder &decoder)
{
    assert(!decoder.readBoolean()); // TODO extension marker
    m_optionals = decoder.readBitset<4>();
    issuingDetail.decode(decoder);
    if (travelerDetailIsSet()) {
        travelerDetail.decode(decoder);
    }
    if (transportDocumentIsSet()) {
        transportDocument = decoder.readSequenceOf<DocumentData>();
    }
}

#include "moc_fcbticket.cpp"
