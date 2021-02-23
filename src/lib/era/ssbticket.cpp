/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ssbticket.h"

#include <QDebug>

#include <cstring>

using namespace KItinerary;

enum {
    SSB_DATA_SIZE = 114,
    SSB_CHAR_WIDTH = 6,
    SSB_VERSION = 3,
};

SSBTicket::SSBTicket() = default;

SSBTicket::SSBTicket(const QByteArray &data)
{
    if (maybeSSB(data)) {
        m_data = data;
    } else {
        qWarning() << "Trying to construct an SSB ticket from invalid data!";
    }
}

SSBTicket::~SSBTicket() = default;

bool SSBTicket::isValid() const
{
    return !m_data.isEmpty();
}

int SSBTicket::readNumber(int start, int length) const
{
    if (start < 0 || length < 1 || start / 8 >= m_data.size() || (start + length) / 8 >= m_data.size() || length > 31) {
        qWarning() << "invalid SSB read:" << start << length;
        return {};
    }

    uint64_t num = 0;
    for (int i = 0; i < 8; ++i) {
        num <<= 8;
        num |= (uint8_t)*(m_data.constData() + (start / 8) + i);
    }
    num <<= start % 8;
    num >>= 64 - length;

    return num;
}

QString SSBTicket::readString(int start, int length) const
{
    QString res;
    res.resize(length);
    for (int i = 0; i < length; ++i) {
        res[i] = QChar(readNumber(start + SSB_CHAR_WIDTH * i, SSB_CHAR_WIDTH) + 32);
    }
    return res;
}

bool SSBTicket::maybeSSB(const QByteArray& data)
{
    if (data.size() != SSB_DATA_SIZE) {
        return false;
    }
    return (data.at(0) >> 4) == SSB_VERSION;
}

QDate SSBTicket::issueDate(const QDate &contextDate)
{
    if (m_data.isEmpty() || ticketTypeCode() > SSBTicket::RPT) {
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

QDate SSBTicket::type1DepartureDay(const QDate& contextDate)
{
    if (ticketTypeCode() != SSBTicket::IRT_RES_BOA) {
        return {};
    }

    const auto d = issueDate(contextDate);
    return d.addDays(type1DepartureDate());
}
