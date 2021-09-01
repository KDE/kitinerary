/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KItinerary/VdvTicketContent>

namespace KItinerary {

#define VDV_NUM_PROPERTY(Name, Size) \
public: \
    VdvNumber<Size> Name; \
    Q_PROPERTY(uint Name MEMBER Name)
#define VDV_DATETIME_PROPERTY(Name) \
public: \
    VdvDateTimeCompact Name; \
    Q_PROPERTY(QDateTime Name MEMBER Name)

#pragma pack(push)
#pragma pack(1)

class KITINERARY_EXPORT Vendor0080VUCommonData
{
    Q_GADGET
    VDV_NUM_PROPERTY(terminalNumber, 2)
    VDV_NUM_PROPERTY(samNumber, 3)
    VDV_NUM_PROPERTY(numberOfPersons, 1)
    VDV_NUM_PROPERTY(numberOfTickets, 1)
};

class KITINERARY_EXPORT Vendor0080VUTicketData
{
    Q_GADGET
    VDV_NUM_PROPERTY(authorizationNumber, 4)
    VDV_NUM_PROPERTY(kvpOrgId, 2)
    VDV_NUM_PROPERTY(productNumber, 2)
    VDV_NUM_PROPERTY(pvOrgId, 2)
    VDV_DATETIME_PROPERTY(validFrom)
    VDV_DATETIME_PROPERTY(validUntil)
    VDV_NUM_PROPERTY(price, 3) // in Euro-Cent
    VDV_NUM_PROPERTY(samSequnceNumber, 4)
    VDV_NUM_PROPERTY(areaListLength, 1)
    uint8_t validityAreaTag; // fixed 0xDC
    uint8_t validityAreaDataSize;
    VdvTicketValidityAreaData validityArea;
};

#pragma pack(pop)

#undef VDV_NUM_PROPERTY
#undef VDV_DATETIME_PROPERTY

}

