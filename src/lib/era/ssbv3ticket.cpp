/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ssbv3ticket.h"

#include <QDebug>

#include <cstring>

using namespace KItinerary;

enum {
    SSB_DATA_SIZE = 114,
    SSB_CHAR_WIDTH = 6,
    SSB_VERSION = 3,
};

SSBv3Ticket::SSBv3Ticket() = default;

SSBv3Ticket::SSBv3Ticket(const QByteArray &data)
{
    if (maybeSSB(data)) {
        m_data = data;
    } else {
        qWarning() << "Trying to construct an SSB ticket from invalid data!";
    }
}

SSBv3Ticket::~SSBv3Ticket() = default;

bool SSBv3Ticket::isValid() const
{
    return !m_data.isEmpty();
}

QString SSBv3Ticket::readString(int start, int length) const
{
    QString res;
    res.resize(length);
    for (int i = 0; i < length; ++i) {
        res[i] = QChar(readNumber(start + SSB_CHAR_WIDTH * i, SSB_CHAR_WIDTH) + 32);
    }
    return res;
}

bool SSBv3Ticket::maybeSSB(const QByteArray& data)
{
    if (data.size() != SSB_DATA_SIZE) {
        return false;
    }
    return (data.at(0) >> 4) == SSB_VERSION;
}

QDate SSBv3Ticket::issueDate(const QDate &contextDate)
{
    if (m_data.isEmpty() || ticketTypeCode() > SSBv3Ticket::RPT) {
        return {};
    }

    int year = contextDate.year();
    if (year % 10 != yearOfIssue()) {
        year += (10 + yearOfIssue() - year % 10) % 10;
    }
    QDate d(year, 1, 1);
    d = d.addDays(issuingDay() - 1);
    return d;
}

QDate SSBv3Ticket::type1DepartureDay(const QDate& contextDate)
{
    if (ticketTypeCode() != SSBv3Ticket::IRT_RES_BOA) {
        return {};
    }

    const auto d = issueDate(contextDate);
    return d.addDays(type1DepartureDate());
}

QByteArray SSBv3Ticket::rawData() const
{
    return m_data;
}
