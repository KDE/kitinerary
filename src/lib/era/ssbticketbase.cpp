/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ssbticketbase.h"

#include <QDateTime>
#include <QDebug>
#include <QString>

using namespace KItinerary;

enum {
    SSB_CHAR_WIDTH = 6,
};

SSBTicketBase::SSBTicketBase() = default;
SSBTicketBase::~SSBTicketBase() = default;

QByteArray SSBTicketBase::rawData() const
{
    return m_data;
}

QByteArray SSBTicketBase::encodedData() const
{
    return m_isBase64 ? m_data.toBase64() : m_data;
}

quint64 SSBTicketBase::readNumber(int start, int length) const
{
    if (start < 0 || length < 1 || start / 8 >= m_data.size() || (start + length) / 8 >= m_data.size() || length > 63) {
        qWarning() << "invalid SSB read:" << start << length;
        return {};
    }

    quint64 num = 0;
    for (int i = 0; i < 8; ++i) {
        num <<= 8;
        num |= (uint8_t)*(m_data.constData() + (start / 8) + i);
    }
    num <<= start % 8;
    num >>= 64 - length;

    return num;
}

QString SSBTicketBase::readString(int start, int length) const
{
    QString res;
    res.reserve(length);
    for (int i = 0; i < length; ++i) {
        auto n = readNumber(start + SSB_CHAR_WIDTH * i, SSB_CHAR_WIDTH);
        if (n >= 36 && n != 63) {
            continue;
        }
        if (n <= 9) {
            res += QLatin1Char(n + '0');
        } else if (n != 63) {
            res += QLatin1Char(n - 10 + 'A');
        } else {
            // outside of the specification, seems to be in use in Finland though
            res += u"Ä";
        }
    }
    return res;
}

QDate SSBTicketBase::dayNumberToDate(int days, const QDateTime &context)
{
    if (days <= 0 || days > 366) {
        return {};
    }

    QDate dt(context.date().year(), 1, 1);
    dt = dt.addDays(days - 1);
    if (dt < context.date()) {
        dt = QDate(context.date().year() + 1, 1, 1);
        dt = dt.addDays(days - 1);
    }
    return dt;
}

#include "moc_ssbticketbase.cpp"
