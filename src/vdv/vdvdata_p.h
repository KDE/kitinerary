/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef KITINERARY_VDVDATA_P_H
#define KITINERARY_VDVDATA_P_H

#include "../tlv/berelement_p.h"

#include <QtEndian>
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

/** Two-digit BCD encoded number. */
struct VdvBcdNumber
{
    uint8_t data;

    uint8_t value() const
    {
        return ((data & 0xF0) >> 4) * 10 + (data & 0x0F);
    }
};

/** Date encoded as 8 BCD digits. */
struct VdvBcdDate
{
    VdvBcdNumber bcdYear[2];
    VdvBcdNumber bcdMonth;
    VdvBcdNumber bcdDay;

    inline uint16_t year() const
    {
        return bcdYear[0].value() * 100 + bcdYear[1].value();
    }
    inline uint8_t month() const
    {
        return bcdMonth.value();
    }
    inline uint8_t day() const
    {
        return bcdDay.value();
    }
};

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

/** Date/time representation encoded in 4 byte. */
struct VdvDateTimeCompact
{
    uint32_t data;

    inline int year() const
    {
        return ((qFromBigEndian(data) & 0b1111'1110'0000'0000'0000'0000'0000'0000) >> 25) + 1990;
    }
    inline int month() const
    {
        return (qFromBigEndian(data) & 0b0000'0001'1110'0000'0000'0000'0000'0000) >> 21;
    }
    inline int day() const
    {
        return (qFromBigEndian(data) & 0b0000'0000'0001'1111'0000'0000'0000'0000) >> 16;
    }
    inline int hour() const
    {
        return (qFromBigEndian(data) & 0b0000'0000'0000'0000'1111'1000'0000'0000) >> 11;
    }
    inline int minute() const
    {
        return (qFromBigEndian(data) & 0b0000'0000'0000'0000'0000'0111'1110'0000) >> 5;
    }
    inline int second() const
    {
        return (qFromBigEndian(data) & 0b0000'0000'0000'0000'0000'0000'0001'1111) * 2;
    }
};

/** Ticket data header. */
struct VdvTicketHeader
{
    uint32_t ticketId;
    uint16_t kvpOrgId;
    uint16_t productId;
    uint16_t pvOrgId;
    VdvDateTimeCompact beginDt;
    VdvDateTimeCompact endDt;
};

/** Product specific data - basic information. */
struct VdvTicketBasicData
{
    uint8_t paymentType;
    uint8_t travelerType; // 1 adult, 2 child, 65 bike
    uint8_t includedTravelerType1; // 0 none, 1 adult, 2 child, 251 family child
    uint8_t includedTravelerCount1;
    uint8_t includedTravelerType2;
    uint8_t includedTravelerCount2;
    uint8_t categroy;
    uint8_t serviceClass; // 1 first class, 2 second class, 3 first class upgrade
    uint8_t price[3]; // 24 bit big endian, price in Euro cent
    uint16_t vat; // VAT rate in 0,01% steps
    uint8_t priceCategory;
    uint8_t productNumber[3];
};

/** Product specific data - traveler information. */
struct VdvTicketTravelerData
{
    uint8_t gender;
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
    uint16_t kvpOrgId;
    uint8_t terminalId[5];
    VdvDateTimeCompact dt;
    uint8_t locationId[6];
};

/** Ticket issuer data block. */
struct VdvTicketIssueData
{
    uint32_t samSeq1;
    uint8_t version;
    uint32_t samSeq2;
    uint8_t samId[3];
};

/** Ticket trailer, after padding. */
struct VdvTicketTrailer
{
    const char identifier[3];
    uint16_t version;
};

#pragma pack(pop)

}

#endif // KITINERARY_VDVDATA_P_H
