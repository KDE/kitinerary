/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ssbv1ticket.h"

#include <QDebug>

#include <cstring>

using namespace KItinerary;

enum {
    SSB_DATA_SIZE_MIN = 107,
    SSB_DATA_SIZE_MAX = 111,
    SSB_VERSION = 1,
};

SSBv1Ticket::SSBv1Ticket() = default;

SSBv1Ticket::SSBv1Ticket(const QByteArray &data)
{
    if (maybeSSB(data)) {
        m_data = data;
    } else {
        qWarning() << "Trying to construct an SSB ticket from invalid data!";
    }
}

SSBv1Ticket::~SSBv1Ticket() = default;

bool SSBv1Ticket::isValid() const
{
    return !m_data.isEmpty();
}

bool SSBv1Ticket::maybeSSB(const QByteArray& data)
{
    if (data.size() < SSB_DATA_SIZE_MIN || data.size() > SSB_DATA_SIZE_MAX) {
        return false;
    }
    return (data.at(0) >> 4) == SSB_VERSION;
}

QByteArray SSBv1Ticket::rawData() const
{
    return m_data;
}
