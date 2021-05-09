/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ssbticketbase.h"

#include <QDebug>
#include <QString>

using namespace KItinerary;

enum {
    SSB_CHAR_WIDTH = 6,
};

SSBTicketBase::SSBTicketBase() = default;
SSBTicketBase::~SSBTicketBase() = default;

int SSBTicketBase::readNumber(int start, int length) const
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

QString SSBTicketBase::readString(int start, int length) const
{
    QString res;
    res.reserve(length);
    for (int i = 0; i < length; ++i) {
        auto n = readNumber(start + SSB_CHAR_WIDTH * i, SSB_CHAR_WIDTH);
        if (n >= 36) {
            continue;
        }
        if (n <= 9) {
            res += QLatin1Char(n + '0');
        } else {
            res += QLatin1Char(n - 10 + 'A');
        }
    }
    return res;
}
