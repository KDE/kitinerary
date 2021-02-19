/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_VDVDATA_P_H
#define KITINERARY_VDVDATA_P_H

#include "../tlv/berelement_p.h"
#include "vdvbasictypes.h"

#include <cstdint>

namespace KItinerary {

enum {
    TagSignature = 0x9E,
    TagSignatureRemainder = 0x9A,
    TagCaReference = 0x42,
    TagTicketProductData = 0x85,
    TagTicketProductTransactionData = 0x8A,
    TagTicketBasicData = 0xDA,
    TagTicketTravelerData = 0xDB,
    TagCertificate = 0x7F21,
    TagCertificateSignature = 0x5F37,
    TagCertificateSignatureRemainder = 0x5F38,
    TagCertificateContent = 0x5F4E,
};

enum {
    MinimumTicketDataSize = 111,
};

#pragma pack(push)
#pragma pack(1)

/** Certificate Authority Reference (CAR) content. */
struct VdvCaReference
{
    char region[2];
    char name[3];
    uint8_t serviceIndicator: 4;
    uint8_t discretionaryData: 4;
    uint8_t algorithmReference;
    uint8_t year;
};

/** Certificate Holder Reference (CHR) */
struct VdvCertificateHolderReference {
    uint8_t filler[4]; // always null
    char name[5];
    uint8_t serviceIndicator: 4;
    uint8_t discretionaryData: 4;
    uint8_t algorithmReference;
    uint8_t year;
};

/** Certificate Holder Authorization (CHA) */
struct VdvCertificateHolderAuthorization {
    char name[6];
    uint8_t stuff;
};

/** Certificate key, contained in a certificate object. */
struct VdvCertificateKey {
    uint8_t certificateProfileIdentifier;
    VdvCaReference car;
    VdvCertificateHolderReference chr;
    VdvCertificateHolderAuthorization cha;
    VdvBcdDate date;
    uint8_t oidBegin;

    inline uint8_t oidSize() const
    {
        return oidBegin == 0x2a ? 9 : 7; // ugly, but works for now
    }

    inline uint8_t headerSize() const
    {
        return sizeof(VdvCertificateKey) + oidSize() - 1;
    }
};

/** Ticket data header. */
struct VdvTicketHeader
{
    VdvNumber<4> ticketId;
    VdvNumber<2> kvpOrgId;
    VdvNumber<2> productId;
    VdvNumber<2> pvOrgId;
    VdvDateTimeCompact beginDt;
    VdvDateTimeCompact endDt;
};

/** Product specific data - basic information. */
struct VdvTicketBasicData
{
    VdvNumber<1> paymentType;
    VdvNumber<1> travelerType; // 1 adult, 2 child, 65 bike
    VdvNumber<1> includedTravelerType1; // 0 none, 1 adult, 2 child, 251 family child
    VdvNumber<1> includedTravelerCount1;
    VdvNumber<1> includedTravelerType2;
    VdvNumber<1> includedTravelerCount2;
    VdvNumber<1> categroy;
    VdvNumber<1> serviceClass; // 1 first class, 2 second class, 3 first class upgrade
    VdvNumber<3> price; // 24 bit big endian, price in Euro cent
    VdvNumber<2> vat; // VAT rate in 0,01% steps
    VdvNumber<1> priceCategory;
    VdvNumber<3> productNumber;
};

/** Product specific data - traveler information. */
struct VdvTicketTravelerData
{
    VdvNumber<1> gender;
    VdvBcdDate birthDate;
    char nameBegin;

    inline const char* name() const
    {
        return &nameBegin;
    }
    inline int nameSize(int elementSize) const
    {
        return elementSize - sizeof(VdvTicketTravelerData) + 1;
    }
};

/** Ticket transaction data block. */
struct VdvTicketTransactionData
{
    VdvNumber<2> kvpOrgId;
    VdvNumber<1> terminalTypeCode;
    VdvNumber<2> terminalNumber;
    VdvNumber<2> terminalOrganizationNumber;
    VdvDateTimeCompact dt;
    VdvNumber<1> locationTypeCode;
    VdvNumber<3> locationNumber;
    VdvNumber<2> locationOrganizationNumber;
};

/** Ticket issuer data block. */
struct VdvTicketIssueData
{
    VdvNumber<4> samSeq1;
    VdvNumber<1> version;
    VdvNumber<4> samSeq2;
    VdvNumber<3> samId;
};

/** Ticket trailer, after padding. */
struct VdvTicketTrailer
{
    const char identifier[3];
    VdvNumber<2> version;
};

#pragma pack(pop)

}

#endif // KITINERARY_VDVDATA_P_H
