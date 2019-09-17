/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "alphaid.h"

using namespace KItinerary;

uint16_t KnowledgeDb::Internal::alphaIdFromString(const QString &s, int size)
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
