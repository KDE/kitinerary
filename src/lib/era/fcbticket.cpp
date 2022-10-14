/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "fcbticket.h"

#include "asn1/uperdecoder.h"

#define FCB_READ_CONSTAINED_INT(Name, Min, Max) \
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

#define FCB_READ_EXTENDED_ENUM(Name) \
    if (Name ## IsSet()) \
        Name = decoder.readEnumeratedWithExtensionMarker<decltype(Name)>()

#define FCB_READ_INT_IA5_PAIR(Name, Min, Max) \
    FCB_READ_CONSTAINED_INT(Name ## Num, Min, Max); \
    FCB_READ_IA5STRING(Name ## IA5)

#define FCB_READ_INT_IA5_PAIR_UNCONSTRAINED(Name) \
    FCB_READ_UNCONSTRAINED_INT(Name ## Num); \
    FCB_READ_IA5STRING(Name ## IA5)

using namespace KItinerary;

void Fcb::IssuingData::decode(UPERDecoder &decoder)
{
    assert(!decoder.readBoolean()); // TODO extension marker
    m_optionals = decoder.readBitset<14>();
    FCB_READ_INT_IA5_PAIR(securityProvider, 1, 32000);
    FCB_READ_INT_IA5_PAIR(issuer, 1, 32000);
    issuingYear = decoder.readConstrainedWholeNumber(2016, 2269);
    issuingDay = decoder.readConstrainedWholeNumber(1, 366);
    FCB_READ_CONSTAINED_INT(issuingTime, 0, 1440);
    FCB_READ_UTF8STRING(issuerName);
    specimen = decoder.readBoolean();
    securePaperTicket = decoder.readBoolean();
    activated = decoder.readBoolean();
    if (currencyIsSet()) {
        currency = decoder.readIA5String(3, 3);
    }
    FCB_READ_CONSTAINED_INT(currencyFract, 1, 3);
    FCB_READ_IA5STRING(issuerPNR);
    assert(!m_optionals[4]); // TODO ExtensionData
    FCB_READ_INT_IA5_PAIR_UNCONSTRAINED(issuedOnTrain);
    FCB_READ_UNCONSTRAINED_INT(issuedOnLine);
    assert(!m_optionals[0]); // TODO GeoCoordinateType
}

void Fcb::TravelerType::decode(UPERDecoder &decoder)
{
    assert(!decoder.readBoolean()); // TODO extension marker

    m_optionals = decoder.readBitset<17>();
    FCB_READ_UTF8STRING(firstName);
    FCB_READ_UTF8STRING(secondName);
    FCB_READ_UTF8STRING(lastName);
    FCB_READ_IA5STRING(idCard);
    FCB_READ_IA5STRING(passportId);
    FCB_READ_IA5STRING(title);
    FCB_READ_EXTENDED_ENUM(gender);
    FCB_READ_IA5STRING(customerIdIA5);
    FCB_READ_UNCONSTRAINED_INT(customerIdNum);
    FCB_READ_CONSTAINED_INT(yearOfBirth, 1901, 2155);
    FCB_READ_CONSTAINED_INT(dayOfBirth, 0, 370);
    ticketHolder = decoder.readBoolean();
    FCB_READ_EXTENDED_ENUM(passengerType);
    if (passengerWithReducedMobilityIsSet()) {
        passengerWithReducedMobility = decoder.readBoolean();
    }
    FCB_READ_CONSTAINED_INT(countryOfResidence, 1, 999);
    FCB_READ_CONSTAINED_INT(countryOfPassport, 1, 999);
    FCB_READ_CONSTAINED_INT(countryOfIdCard, 1, 999);
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
    FCB_READ_UTF8STRING(groupName);
}

void Fcb::TrainLinkType::decode(UPERDecoder &decoder)
{
    m_optionals = decoder.readBitset<9>();
    FCB_READ_INT_IA5_PAIR_UNCONSTRAINED(train);
    travelDate = decoder.readConstrainedWholeNumber(-1, 370);
    departureTime = decoder.readConstrainedWholeNumber(0, 1440);
    FCB_READ_CONSTAINED_INT(departureUTCOffset, -60, 60);
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
    // TODO
    assert(false);
}

void Fcb::OpenTicketData::decode(UPERDecoder &decoder)
{
    assert(!decoder.readBoolean()); // TODO extension marker

    m_optionals = decoder.readBitset<38>();
    FCB_READ_INT_IA5_PAIR_UNCONSTRAINED(reference);
    FCB_READ_INT_IA5_PAIR(productOwner, 1, 32000);
    FCB_READ_INT_IA5_PAIR(productId, 0, 32000);
    FCB_READ_UNCONSTRAINED_INT(extIssuerId);
    FCB_READ_UNCONSTRAINED_INT(issuerAutorizationId);
    returnIncluded = decoder.readBoolean();
    if (stationCodeTableIsSet()) {
        stationCodeTable = decoder.readEnumerated<CodeTableType>();
    }
    FCB_READ_INT_IA5_PAIR(fromStation, 1, 9999999);
    FCB_READ_INT_IA5_PAIR(toStation, 1, 9999999);
    FCB_READ_UTF8STRING(fromStationNameUTF8);
    FCB_READ_UTF8STRING(toStationNameUTF8);
    FCB_READ_UTF8STRING(validRegionDesc);
    if (validRegionIsSet()) {
        validRegion = decoder.readSequenceOf<RegionalValidityType>();
    }
    if (returnDescriptionIsSet()) {
        returnDescription.decode(decoder);
    }
    FCB_READ_CONSTAINED_INT(validFromDay, -1, 700);
    FCB_READ_CONSTAINED_INT(validFromTime, 0, 1440);
    FCB_READ_CONSTAINED_INT(validFromUTCOffset, -60, 60);
    FCB_READ_CONSTAINED_INT(validUntilDay, 0, 370);
    FCB_READ_CONSTAINED_INT(validUntilTime, 0, 1440);
    FCB_READ_CONSTAINED_INT(validUntilUTCOffset, -60, 60);
    assert(!activatedDayIsSet()); // TODO activatedDay
    if (classCodeIsSet()) {
        classCode = decoder.readEnumeratedWithExtensionMarker<TravelClassType>();
    }
    FCB_READ_IA5STRING(serviceLevel);
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
