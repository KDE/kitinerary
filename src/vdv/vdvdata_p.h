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

#include <QtEndian>
#include <cstdint>

namespace KItinerary {

enum : uint8_t {
    TagSignature = 0x9E,
    TagSignatureRemainder = 0x9A,
    TagCaReference = 0x42,
    TagOneByteSize = 0x81,
    TagTwoByteSize = 0x82,
    TagTicketProductData = 0x85,
    TagTicketProductTransactionData = 0x8A,
    TagTicketBasicData = 0xDA,
    TagTicketTravelerData = 0xDB,
};

enum : uint16_t {
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

/** Generic structure for the TLV (tag-length-value) data blocks in VDV binary data.
 *  This consits of:
 *  - a one or two byte tag (@tparam TagType) with a fixed value (@tparam TagValue)
 *  - a one byte field indicating the size of the size field (optional)
 *  - one or two bytes for the size
 *  - followed by size bytes of content
 */
template <typename TagType>
struct VdvTlvBlock
{
    TagType tag;
    uint8_t size0;

    inline uint16_t contentSize() const
    {
        return size0;
    }

    inline uint16_t contentOffset() const
    {
        return sizeof(VdvTlvBlock);
    }

    inline const uint8_t* contentData() const
    {
        return reinterpret_cast<const uint8_t*>(this) + contentOffset();
    }

    inline uint16_t size() const
    {
        return contentSize() + contentOffset();
    }

    template <typename T>
    inline const T* contentAt(int offset) const
    {
        return reinterpret_cast<const T*>(contentData() + offset);
    }
};

template <typename TagType, TagType TagValue>
struct VdvTlvTypedBlock : public VdvTlvBlock<TagType>
{
    enum : TagType { Tag = TagValue };
    inline bool isValid() const
    {
        return qFromBigEndian(VdvTlvBlock<TagType>::tag) == TagValue;
    }
};

template <typename TagType, TagType TagValue>
struct VdvTaggedSizeDataBlock
{
    TagType tag;
    uint8_t sizeTag;
    uint8_t size0;
    uint8_t size1;

    inline bool isValid() const
    {
        return qFromBigEndian(tag) == TagValue && (sizeTag == TagOneByteSize || sizeTag == TagTwoByteSize);
    }

    inline uint16_t contentSize() const
    {
        return sizeTag == TagOneByteSize ? size0 : ((size0 << 8) + size1);
    }

    inline uint16_t contentOffset() const
    {
        return sizeof(VdvTaggedSizeDataBlock) - ((sizeTag == TagOneByteSize) ? 1 : 0);
    }

    inline const uint8_t* contentData() const
    {
        return reinterpret_cast<const uint8_t*>(this) + contentOffset();
    }

    inline uint16_t size() const
    {
        return contentSize() + contentOffset();
    }

    template <typename T>
    inline const T* contentAt(int offset) const
    {
        return reinterpret_cast<const T*>(contentData() + offset);
    }
};

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

/** Signature container for the signed part of the payload data. */
struct VdvSignature : public VdvTaggedSizeDataBlock<uint8_t, TagSignature> {};
/** Signature Remainder header. */
struct VdvSignatureRemainder : public VdvTlvTypedBlock<uint8_t, TagSignatureRemainder> {};
/** CV certificate. */
struct VdvCertificateHeader : public VdvTaggedSizeDataBlock<uint16_t, TagCertificate> {};

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
struct VdvCaReferenceBlock : public VdvTlvTypedBlock<uint8_t, TagCaReference> {};

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
struct VdvCertificateKeyBlock : public VdvTaggedSizeDataBlock<uint16_t, TagCertificateContent> {};

/** Certificate signature. */
struct VdvCertificateSignature : public VdvTaggedSizeDataBlock<uint16_t, TagCertificateSignature> {};
/** Certificate signature remainder. */
struct VdvCertificateSignatureRemainder : public VdvTlvTypedBlock<uint16_t, TagCertificateSignatureRemainder> {};

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

/** Product-specific ticket data block.
 *  Contains a set of TLV elements.
 */
struct VdvTicketProductData : public VdvTlvTypedBlock<uint8_t, TagTicketProductData>
{
    /** First TLV element inside this block. */
    const VdvTlvBlock* first() const
    {
        if (contentSize() < 2) {
            return nullptr;
        }
        const auto tlv = contentAt<VdvTlvBlock>(0);
        return tlv->size() > contentSize() ? nullptr : tlv;
    }
    /** Next TLV element, for iteration. */
    const VdvTlvBlock* next(const VdvTlvBlock *prev) const
    {
        const auto off = (const uint8_t*)prev - contentData() + prev->size();
        if (off + 2 > contentSize()) {
            return nullptr;
        }
        const auto tlv = contentAt<VdvTlvBlock>(off);
        return tlv->size() > contentSize() ? nullptr : tlv;
    }

    /** Find TLV element with type @tparam T. */
    template <typename T>
    inline const T* contentByTag() const
    {
        for (auto tlv = first(); tlv; tlv = next(tlv)) {
            if (qFromBigEndian(tlv->tag) == T::Tag && tlv->size() >= sizeof(T)) {
                return reinterpret_cast<const T*>(tlv);
            }
        }
        return nullptr;
    }
};

/** Product specific data - basic information. */
struct VdvTicketBasicData : public VdvTlvTypedBlock<uint8_t, TagTicketBasicData>
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
struct VdvTicketTravelerData : public VdvTlvTypedBlock<uint8_t, TagTicketTravelerData>
{
    uint8_t gender;
    VdvBcdDate birthDate;
    char nameBegin;

    inline const char* name() const
    {
        return &nameBegin;
    }
    inline int nameSize() const
    {
        return size() - sizeof(VdvTicketTravelerData) + 1;
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

/** Product-specific transaction data block (variable length). */
struct VdvTicketProductTransactionData : public VdvTlvTypedBlock<uint8_t, TagTicketProductTransactionData> {};

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
