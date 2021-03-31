/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "vdvbasictypes.h"

namespace KItinerary {

enum {
    TagSignature = 0x9E,
    TagSignatureRemainder = 0x9A,
    TagCaReference = 0x42,
    TagTicketProductData = 0x85,
    TagTicketProductTransactionData = 0x8A,
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

#pragma pack(pop)

}

