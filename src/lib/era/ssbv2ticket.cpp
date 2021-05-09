/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ssbv2ticket.h"

#include <QDebug>

using namespace KItinerary;

enum {
    SSB_DATA_SIZE_MIN = 67,
    SSB_DATA_SIZE_MAX = 114,
    SSB_VERSION = 2,
};

SSBv2Ticket::SSBv2Ticket() = default;

SSBv2Ticket::SSBv2Ticket(const QByteArray &data)
{
    if (maybeSSB(data)) {
        m_data = data;
    } else {
        qWarning() << "Trying to construct an SSB ticket from invalid data!";
    }
}

SSBv2Ticket::~SSBv2Ticket() = default;

bool SSBv2Ticket::isValid() const
{
    return !m_data.isEmpty();
}

bool SSBv2Ticket::maybeSSB(const QByteArray& data)
{
    if (data.size() < SSB_DATA_SIZE_MIN || data.size() > SSB_DATA_SIZE_MAX) {
        return false;
    }
    return (data.at(0) >> 4) == SSB_VERSION;
}

QByteArray SSBv2Ticket::rawData() const
{
    return m_data;
}
