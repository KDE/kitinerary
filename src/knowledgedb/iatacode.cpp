/*
   Copyright (c) 2017-2019 Volker Krause <vkrause@kde.org>

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   This library is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
   License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.
*/

#include "iatacode.h"

#include <QDebug>

QDebug operator<<(QDebug dbg, KItinerary::KnowledgeDb::IataCode code)
{
    dbg << code.toString();
    return dbg;
}

namespace KItinerary {
namespace KnowledgeDb {

static_assert(sizeof(IataCode) == sizeof(uint16_t), "IATA code changed size!");

IataCode::IataCode(const QString &iataStr) : IataCode()
{
    if (iataStr.size() != 3) {
        return;
    }
    if (!iataStr.at(0).isUpper() || !iataStr.at(1).isUpper() || !iataStr.at(2).isUpper()) {
        return;
    }
    m_letter0 = iataStr.at(0).toLatin1() - 'A';
    m_letter1 = iataStr.at(1).toLatin1() - 'A';
    m_letter2 = iataStr.at(2).toLatin1() - 'A';
    m_valid = 1;
}

bool IataCode::isValid() const
{
    return m_valid;
}

bool IataCode::operator<(IataCode rhs) const
{
    return toUInt16() < rhs.toUInt16();
}

bool IataCode::operator==(IataCode other) const
{
    return memcmp(this, &other, sizeof(IataCode)) == 0;
}

bool IataCode::operator!=(IataCode other) const
{
    return memcmp(this, &other, sizeof(IataCode)) != 0;
}

QString IataCode::toString() const
{
    if (!isValid()) {
        return QString();
    }
    QString s;
    s.reserve(3);
    s.push_back(QLatin1Char(m_letter0 + 'A'));
    s.push_back(QLatin1Char(m_letter1 + 'A'));
    s.push_back(QLatin1Char(m_letter2 + 'A'));
    return s;
}

uint16_t IataCode::toUInt16() const
{
    return m_letter0 << 11 | m_letter1 << 6 | m_letter2 << 1 | m_valid;
}

}}
