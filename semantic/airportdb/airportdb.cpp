/*
   Copyright (c) 2017 Volker Krause <vkrause@kde.org>

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

#include "airportdb.h"
#include "airportdb_p.h"
#include "airportdb_p.cpp"

#include <QDebug>
#include <QRegularExpression>
#include <QTimeZone>

#include <algorithm>
#include <cstring>

namespace AirportDb {
static_assert(sizeof(IataCode) == sizeof(uint16_t), "IATA code changed size!");
static constexpr auto iata_table_size = sizeof(iata_table) / sizeof(IataCode);
static_assert(iata_table_size == sizeof(coordinate_table) / sizeof(Coordinate), "Airport coordinate table size mismatch!");
static_assert(iata_table_size == sizeof(timezone_table) / sizeof(uint16_t), "Airport timezone table size mismatch!");

bool Coordinate::isValid() const
{
    return !std::isnan(latitude) && !std::isnan(longitude);
}

bool Coordinate::operator==(const Coordinate &other) const
{
    return latitude == other.latitude && longitude == other.longitude;
}

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

static const IataCode *iataBegin()
{
    return iata_table;
}

static const IataCode *iataEnd()
{
    return iata_table + iata_table_size;
}

static int indexOfAirport(IataCode iataCode)
{
    const auto iataIt = std::lower_bound(iataBegin(), iataEnd(), iataCode);
    if (iataIt == iataEnd() || (*iataIt) != iataCode) {
        return -1;
    }

    return std::distance(iataBegin(), iataIt);
}

Coordinate coordinateForAirport(IataCode iataCode)
{
    const auto iataIdx = indexOfAirport(iataCode);
    if (iataIdx >= 0) {
        return coordinate_table[iataIdx];
    }
    return {};
}

QTimeZone timezoneForAirport(IataCode iataCode)
{
    const auto iataIdx = indexOfAirport(iataCode);
    if (iataIdx < 0) {
        return QTimeZone();
    }
    return QTimeZone(timezone_names + timezone_table[iataIdx]);
}

static const auto name_string_index_size = sizeof(name_string_index) / sizeof(NameIndex);

static const NameIndex *nameIndexBegin()
{
    return name_string_index;
}

static const NameIndex *nameIndexEnd()
{
    return name_string_index + name_string_index_size;
}

IataCode iataCodeFromName(const QString &name)
{
    IataCode code;
    int iataIdx = -1;
    for (const auto &s : name.toCaseFolded().split(QRegularExpression(QStringLiteral("[ 0-9/'\"\\(\\)&\\,.–„-]")), QString::SkipEmptyParts)) {
        const auto it = std::lower_bound(nameIndexBegin(), nameIndexEnd(), s.toUtf8(), [](const NameIndex &lhs, const QByteArray &rhs) {
                const auto cmp = strncmp(name_string_table + lhs.offset(), rhs.constData(), std::min<int>(lhs.length, rhs.size()));
                if (cmp == 0) {
                    return lhs.length < rhs.size();
                }
                return cmp < 0;
            });
        if (it == nameIndexEnd() || it->length != s.toUtf8().size() || strncmp(name_string_table + it->offset(), s.toUtf8().constData(), it->length) != 0) {
            continue;
        }
        if (iataIdx >= 0 && iataIdx != it->iataIndex) {
            return code; // not unique
        }
        iataIdx = it->iataIndex;
    }

    if (iataIdx > 0) {
        code = iata_table[iataIdx];
    }
    return code;
}
}
