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
};

enum : uint16_t {
    TagCvCertificate = 0x7F21,
    TagCvCertificateSignature = 0x5F37,
    TagCvCertificateContent = 0x5F4E,
};

#pragma pack(push)
#pragma pack(1)

/** Signature container for the signed part of the payload data. */
struct VdvSignature {
    uint8_t tag;
    uint8_t stuff; // always 0x81
    uint8_t size;  // always 0x80
    uint8_t data[128];
};

/** Signature Remainder header. */
struct VdvSignatureRemainder {
    enum { Offset = 131 };

    uint8_t tag;
    uint8_t contentSize; // >= 5
    // followed by size bytes with the remainder of the signed payload data. */

    inline bool isValid() const
    {
        return tag == TagSignatureRemainder && contentSize >= 5;
    }

    inline uint8_t size() const
    {
        return contentSize + sizeof(tag) + sizeof(contentSize);
    }
};

/** CV certificate. */
struct VdvCvCertificate {
    uint16_t tag;
    uint8_t size0;
    uint8_t size1;

    inline bool isValid() const
    {
        return  qFromBigEndian(tag) == TagCvCertificate;
    }

    inline uint16_t contentSize() const
    {
        return ((size0 << 8) | size1) - 0x8100;
    }

    inline uint16_t size() const
    {
        return contentSize() + sizeof(tag) + sizeof(size0) + sizeof(size1);
    }
};

/** Certificate Authority Reference (CAR) */
struct VdvCAReference {
    uint8_t tag;
    uint8_t contentSize;
    char region[2];
    char name[3];
    uint8_t serviceIndicator: 4;
    uint8_t discretionaryData: 4;
    uint8_t algorithmReference;
    uint8_t year;

    inline bool isValid() const
    {
        return tag == TagCaReference && contentSize == 8;
    }
};

/** Certificate Holder Reference (CHR) */
struct VdvCertificateHolderReference {
    uint8_t filler[4]; // always null
    char name[5];
    uint8_t extension[3];
};

/** Certificate Holder Authorization (CHA) */
struct VdvCertificateHolderAuthorization {
    char name[6];
    uint8_t stuff;
};

/** Certificate key, contained in a certificate object. */
struct VdvCertificateKey {
    uint16_t tag;
    uint16_t taggedSize;
    uint8_t cpi;
    VdvCAReference car;
    VdvCertificateHolderReference chr;
    VdvCertificateHolderAuthorization cha;
    uint8_t date[3];
    uint8_t oid[9];
    uint8_t modulusBegin;

    inline bool isValid() const
    {
        return qFromBigEndian(tag) == TagCvCertificateContent;
    }
};

/** Certificate signature. */
struct VdvCertificateSignature {
    uint16_t tag;
    uint16_t taggedSize;

    inline bool isValid() const
    {
        return qFromBigEndian(tag) == TagCvCertificateSignature;
    }
};

#pragma pack(pop)

}

#endif // KITINERARY_VDVDATA_P_H
