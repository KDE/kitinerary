/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ssbv2ticket.h"

#include <QDebug>

using namespace KItinerary;

enum {
    SSBV2_DATA_SIZE_MIN = 67,
    SSBV2_DATA_SIZE_MAX = 114,
    SSBV2_VERSION = 2,
};

SSBv2Ticket::SSBv2Ticket() = default;

SSBv2Ticket::SSBv2Ticket(const QByteArray &data, bool isBase64)
{
    if (maybeSSB(data)) {
        m_data = data;
        m_isBase64 = isBase64;

        // additional sanity checking to catch the maybeSSB heuristic not being good enough
        // trainNumber() > 99999 would also be an effective check, wouldn't it be for the Trenitalia
        // deviations from the SSBv2 spec...
        if (numberOfAdultPassengers() > 99 || numberOfChildPassengers() > 99)
        {
            m_data.clear();
        }
    }

    if (m_data.isEmpty()) {
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
    if (data.size() < SSBV2_DATA_SIZE_MIN || data.size() > SSBV2_DATA_SIZE_MAX) {
        return false;
    }
    return (data.at(0) >> 4) == SSBV2_VERSION;
}

QDate SSBv2Ticket::firstDayOfValidity(const QDateTime &contextDate) const
{
    if (!isValid()) {
        return {};
    }
    return dayNumberToDate(firstDayOfValidityDay(), contextDate);
}

QDate SSBv2Ticket::lastDayOfValidity(const QDateTime &contextDate) const
{
    if (!isValid()) {
        return {};
    }
    return dayNumberToDate(lastDayOfValidityDay(), contextDate);
}

#include "moc_ssbv2ticket.cpp"
