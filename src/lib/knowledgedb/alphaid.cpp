/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "alphaid.h"

using namespace KItinerary;

uint16_t KnowledgeDb::Internal::alphaIdFromString(QStringView s, int size)
{
    uint16_t res = {};
    if (size != s.size()) {
        return res;
    }

    for (int i = 0; i < size; ++i) {
        const auto c = s[i];
        if (c.row() != 0 || (c.cell() < 'A' || c.cell() > 'Z')) {
            return {};
        }
        res |= (c.cell() - '@') << (5 * (size - i - 1));
    }

    return res;
}

QString KnowledgeDb::Internal::alphaIdToString(uint16_t id, int size)
{
    if (id == 0) {
        return {};
    }

    QString s;
    s.reserve(size);
    for (int i = 0; i < size; ++i) {
        const auto shift = 5 * (size - i - 1);
        const auto mask = 31;
        const auto c = (id & (mask << shift)) >> shift;
        s.push_back(QLatin1Char(c + '@'));
    }

    return s;
}
