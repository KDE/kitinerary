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
    const auto optionals = decoder.readBitset<14>();
    if (optionals[13]) {
        securityProviderNum = decoder.readConstrainedWholeNumber(1, 32000);
    }
    if (optionals[12]) {
        securityProviderIA5 = decoder.readIA5String();
    }
    if (optionals[11]) {
        issuerNum = decoder.readConstrainedWholeNumber(1, 32000);
    }
    if (optionals[10]) {
        issuerIA5 = decoder.readIA5String();
    }
    issuingYear = decoder.readConstrainedWholeNumber(2016, 2269);
    issuingDay = decoder.readConstrainedWholeNumber(1, 366);
    if (optionals[9]) {
        issuingTime = decoder.readConstrainedWholeNumber(0, 1440);
    }
    if (optionals[8]) {
        issuerName = decoder.readUtf8String();
    }
    specimen = decoder.readBoolean();
    securePaperTicket = decoder.readBoolean();
    activated = decoder.readBoolean();
    if (optionals[7]) {
        currency = decoder.readIA5String(3, 3);
    }
    if (optionals[6]) {
        currencyFract = decoder.readConstrainedWholeNumber(1, 3);
    }
    if (optionals[5]) {
        issuerPNR = decoder.readIA5String();
    }
    assert(!optionals[4]); // TODO ExtensionData
    assert(!optionals[3]); // TODO unconstrained number
    if (optionals[2]) {
        issuedOnTrainIA5 = decoder.readIA5String();
    }
    assert(!optionals[1]); // TODO unconstrained number
    assert(!optionals[0]); // TODO GeoCoordinateType
}

void Fcb::TravelerType::decode(UPERDecoder &decoder)
{
    assert(!decoder.readBoolean()); // TODO extension marker

    const auto optionals = decoder.readBitset<17>();
    if (optionals[16]) {
        firstName = decoder.readUtf8String();
    }
    if (optionals[15]) {
        secondName = decoder.readUtf8String();
    }
    if (optionals[14]) {
        lastName = decoder.readUtf8String();
    }
    if (optionals[13]) {
        idCard = decoder.readIA5String();
    }
    if (optionals[12]) {
        passportId = decoder.readIA5String();
    }
    if (optionals[11]) {
        title = decoder.readIA5String(1, 3);
    }
    assert(!optionals[10]); // TODO gender
    if (optionals[9]) {
        customerIdIA5 = decoder.readIA5String();
    }
    if (optionals[8]) {
        assert(false);
        // TODO read unconstrained integer
    }
    if (optionals[7]) {
        yearOfBirth = decoder.readConstrainedWholeNumber(1901, 2155);
    }
    if (optionals[6]) {
        dayOfBirth = decoder.readConstrainedWholeNumber(0, 370);
    }
    ticketHolder = decoder.readBoolean();
    assert(!optionals[5]); // TODO passengerType
    assert(!optionals[4]); // TODO passengerWithReducedMobility
    if (optionals[3]) {
        countryOfResidence = decoder.readConstrainedWholeNumber(1, 999);
    }
    if (optionals[2]) {
        countryOfPassport = decoder.readConstrainedWholeNumber(1, 999);
    }
    if (optionals[1]) {
        countryOfIdCard = decoder.readConstrainedWholeNumber(1, 999);
    }
    assert(!optionals[0]); // TODO status
}

void Fcb::TravelerData::decode(UPERDecoder &decoder)
{
    assert(!decoder.readBoolean()); // TODO extension marker

    const auto optionals = decoder.readBitset<3>();
    if (optionals[2]) {
        traveler = decoder.readSequenceOf<TravelerType>();
    }
    if (optionals[1]) {
        preferredLanguage = decoder.readIA5String(2, 2);
    }
    if (optionals[0]) {
        groupName = decoder.readUtf8String();
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
    const auto optionals = decoder.readBitset<4>();
    if (optionals[3]) {
        issuingDetail.decode(decoder);
    }
    if (optionals[2]) {
        travelerDetail.decode(decoder);
    }
}
