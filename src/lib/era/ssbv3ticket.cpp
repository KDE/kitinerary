/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ssbv3ticket.h"

#include <QDebug>

#include <cstring>

using namespace KItinerary;

enum {
    SSBV3_MIN_DATA_SIZE = 114,
    SSBV3_MAX_DATA_SIZE = 122,
    SSBV3_CHAR_WIDTH = 6,
    SSBV3_VERSION = 3,
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
        res[i] = QLatin1Char((uint8_t)readNumber(start + SSBV3_CHAR_WIDTH * i, SSBV3_CHAR_WIDTH) + 32);
    }
    return res;
}

bool SSBv3Ticket::maybeSSB(const QByteArray& data)
{
    if (data.size() < SSBV3_MIN_DATA_SIZE || data.size() > SSBV3_MAX_DATA_SIZE) {
        return false;
    }
    return (data.at(0) >> 4) == SSBV3_VERSION;
}

QDate SSBv3Ticket::issueDate(const QDateTime &contextDate) const
{
    if (m_data.isEmpty() || ticketTypeCode() > SSBv3Ticket::RPT) {
        return {};
    }

    int year = contextDate.date().year();
    if (year % 10 != yearOfIssue()) {
        year += (10 + yearOfIssue() - year % 10) % 10;
    }
    QDate d(year, 1, 1);
    d = d.addDays(issuingDay() - 1);
    return d;
}

QDate SSBv3Ticket::type1DepartureDay(const QDateTime& contextDate) const
{
    if (ticketTypeCode() != SSBv3Ticket::IRT_RES_BOA) {
        return {};
    }

    const auto d = issueDate(contextDate);
    return d.addDays(type1DepartureDate());
}

QDate SSBv3Ticket::type2ValidFrom(const QDateTime &contextDate) const
{
    if (ticketTypeCode() != SSBv3Ticket::NRT) {
        return {};
    }

    return issueDate(contextDate).addDays(type2FirstDayOfValidity());
}

QDate SSBv3Ticket::type2ValidUntil(const QDateTime& contextDate) const
{
    if (ticketTypeCode() != SSBv3Ticket::NRT) {
        return {};
    }

    return issueDate(contextDate).addDays(type2LastDayOfValidity());
}

QByteArray SSBv3Ticket::rawData() const
{
    return m_data;
}

#include "moc_ssbv3ticket.cpp"
