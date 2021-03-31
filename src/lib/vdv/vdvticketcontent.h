/*
    SPDX-FileCopyrightText: 2019-2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kitinerary_export.h"
#include "vdvbasictypes.h"

#include <qobjectdefs.h>

namespace KItinerary {

#define VDV_NUM_PROPERTY(Name, Size) \
public: \
    VdvNumber<Size> Name; \
    Q_PROPERTY(uint Name MEMBER Name)
#define VDV_DATETIME_PROPERTY(Name) \
public: \
    VdvDateTimeCompact Name; \
    Q_PROPERTY(QDateTime Name MEMBER Name)
#define VDV_DATE_PROPERTY(Name) \
public: \
    VdvBcdDate Name; \
    Q_PROPERTY(QDate Name MEMBER Name)

#pragma pack(push)
#pragma pack(1)

/** Ticket data header. */
class KITINERARY_EXPORT VdvTicketHeader
{
    Q_GADGET
    VDV_NUM_PROPERTY(ticketId, 4)
    VDV_NUM_PROPERTY(kvpOrgId, 2)
    VDV_NUM_PROPERTY(productId, 2)
    VDV_NUM_PROPERTY(pvOrgId, 2)
    VDV_DATETIME_PROPERTY(validityBegin)
    VDV_DATETIME_PROPERTY(validityEnd)
};

/** Product specific data - basic information. */
class KITINERARY_EXPORT VdvTicketBasicData
{
    Q_GADGET
    VDV_NUM_PROPERTY(paymentType, 1)
    VDV_NUM_PROPERTY(travelerType, 1) // 1 adult, 2 child, 65 bike
    VDV_NUM_PROPERTY(includedTravelerType1, 1) // 0 none, 1 adult, 2 child, 251 family child
    VDV_NUM_PROPERTY(includedTravelerCount1, 1)
    VDV_NUM_PROPERTY(includedTravelerType2, 1)
    VDV_NUM_PROPERTY(includedTravelerCount2, 1)
    VDV_NUM_PROPERTY(categroy, 1)
    VDV_NUM_PROPERTY(serviceClass, 1) // 1 first class, 2 second class, 3 first class upgrade
    VDV_NUM_PROPERTY(price, 3) // 24 bit big endian, price in Euro cent
    VDV_NUM_PROPERTY(vat, 2) // VAT rate in 0,01% steps
    VDV_NUM_PROPERTY(priceCategory, 1)
    VDV_NUM_PROPERTY(productNumber, 3)
public:
    enum { Tag = 0xDA };
};

/** Product specific data - traveler information. */
class KITINERARY_EXPORT VdvTicketTravelerData
{
    Q_GADGET
    VDV_NUM_PROPERTY(gender, 1)
    VDV_DATE_PROPERTY(birthDate)
public:
    char nameBegin;

    inline const char* name() const
    {
        return &nameBegin;
    }
    inline int nameSize(int elementSize) const
    {
        return elementSize - sizeof(VdvTicketTravelerData) + 1;
    }

    enum { Tag = 0xDB };
};

/** Ticket validity area data block. */
class KITINERARY_EXPORT VdvTicketValidityAreaData
{
    Q_GADGET
    VDV_NUM_PROPERTY(type, 1)
    VDV_NUM_PROPERTY(orgId, 2)

public:
    enum { Tag = 0xDC };
};

class KITINERARY_EXPORT VdvTicketValidityAreaDataType31 : public VdvTicketValidityAreaData
{
    Q_GADGET
    VDV_NUM_PROPERTY(startId, 3)
    VDV_NUM_PROPERTY(destinationId, 3)
    VDV_NUM_PROPERTY(wayTextId, 2)
    VDV_NUM_PROPERTY(ticketRelation, 4)
    VDV_NUM_PROPERTY(pointCloudId, 4)

public:
    enum { Type = 0x31 };
};

/** Ticket transaction data block. */
class KITINERARY_EXPORT VdvTicketCommonTransactionData
{
    Q_GADGET
    VDV_NUM_PROPERTY(kvpOrgId, 2)
    VDV_NUM_PROPERTY(terminalTypeCode, 1)
    VDV_NUM_PROPERTY(terminalNumber, 2)
    VDV_NUM_PROPERTY(terminalOrganizationNumber, 2)
    VDV_DATETIME_PROPERTY(transactionDateTime)
    VDV_NUM_PROPERTY(locationTypeCode, 1)
    VDV_NUM_PROPERTY(locationNumber, 3)
    VDV_NUM_PROPERTY(locationOrganizationNumber, 2)
};

/** Ticket issuer data block. */
class KITINERARY_EXPORT VdvTicketIssueData
{
    Q_GADGET
    VDV_NUM_PROPERTY(samSeq1, 4)
    VDV_NUM_PROPERTY(version, 1)
    VDV_NUM_PROPERTY(samSeq2, 4)
    VDV_NUM_PROPERTY(samId, 3)
};

/** Ticket trailer, after padding. */
struct VdvTicketTrailer
{
    const char identifier[3];
    VdvNumber<2> version;
};

#pragma pack(pop)

#undef VDV_NUM_PROPERTY
#undef VDV_DATETIME_PROPERTY
#undef VDV_DATE_PROPERTY

}

