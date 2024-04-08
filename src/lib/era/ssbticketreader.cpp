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

[[nodiscard]] static bool isBase64Char(char c)
{
    return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '=' || c == '/' || c == '+';
}

[[nodiscard]] static bool maybeBase64(QByteArrayView data)
{
    return std::all_of(data.begin(), data.end(), isBase64Char);
}

bool SSBTicketReader::maybeSSB(const QByteArray &data)
{
    if (SSBv1Ticket::maybeSSB(data) || SSBv2Ticket::maybeSSB(data) || SSBv3Ticket::maybeSSB(data)) {
        return true;
    }

    // base64 encoded tickets are found in the wild, although that isn't specified anywhere AFAIK
    if (data.size() > 168 || !maybeBase64(data)) { // 4/3 * largest SSB ticket size
        return false;
    }

    const auto decoded = QByteArray::fromBase64(data);
    return SSBv1Ticket::maybeSSB(decoded) || SSBv2Ticket::maybeSSB(decoded) || SSBv3Ticket::maybeSSB(decoded);
}

[[nodiscard]] static QVariant readInternal(QByteArray ticketData, int versionOverride)
{
    auto version = ticketData[0] >> 4;
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

QVariant SSBTicketReader::read(const QByteArray& data, int versionOverride)
{
    if (data.isEmpty()) {
        return {};
    }

    auto ticket = readInternal(data, versionOverride);
    if (ticket.isValid() || data.size() > 168 || !maybeBase64(data)) {
        return ticket;
    }

    // try base64, see above
    return readInternal(QByteArray::fromBase64(data), versionOverride);
}
