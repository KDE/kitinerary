/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ssbticketreader.h"
#include "ssbv1ticket.h"
#include "ssbv2ticket.h"
#include "ssbv3ticket.h"

#include <QVariant>

using namespace KItinerary;

QVariant SSBTicketReader::read(const QByteArray& data, int versionOverride)
{
    if (data.isEmpty()) {
        return {};
    }

    auto ticketData = data;
    auto version = data[0] >> 4;
    if (versionOverride > 0 && version != versionOverride) {
        ticketData[0] = (ticketData[0] & 0x0f) | (versionOverride << 4);
        version = versionOverride;
    }

    switch (version) {
        case 1:
        {
            SSBv1Ticket ticket(ticketData);
            return ticket.isValid() ? QVariant::fromValue(ticket) : QVariant();
        }
        case 2:
        {
            SSBv2Ticket ticket(ticketData);
            return ticket.isValid() ? QVariant::fromValue(ticket) : QVariant();
        }
        case 3:
        {
            SSBv3Ticket ticket(ticketData);
            return ticket.isValid() ? QVariant::fromValue(ticket) : QVariant();
        }
    }

    return {};
}
