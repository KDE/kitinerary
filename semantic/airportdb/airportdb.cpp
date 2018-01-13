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

static const auto name1_string_index_size = sizeof(name1_string_index) / sizeof(Name1Index);

static const Name1Index *name1IndexBegin()
{
    return name1_string_index;
}

static const Name1Index *name1IndexEnd()
{
    return name1_string_index + name1_string_index_size;
}

static const auto nameN_string_index_size = sizeof(nameN_string_index) / sizeof(NameNIndex);

static const NameNIndex *nameNIndexBegin()
{
    return nameN_string_index;
}

static const NameNIndex *nameNIndexEnd()
{
    return nameN_string_index + nameN_string_index_size;
}

static IataCode iataCodeForUniqueFragment(const QStringList &fragments)
{
    int iataIdx = -1;
    for (const auto &s : fragments) {
        const auto it = std::lower_bound(name1IndexBegin(), name1IndexEnd(), s.toUtf8(), [](const Name1Index &lhs, const QByteArray &rhs) {
                const auto cmp = strncmp(name1_string_table + lhs.offset(), rhs.constData(), std::min<int>(lhs.length, rhs.size()));
                if (cmp == 0) {
                    return lhs.length < rhs.size();
                }
                return cmp < 0;
            });
        if (it == name1IndexEnd() || it->length != s.toUtf8().size() || strncmp(name1_string_table + it->offset(), s.toUtf8().constData(), it->length) != 0) {
            continue;
        }
        if (iataIdx >= 0 && iataIdx != it->iataIndex) {
            return {}; // not unique
        }
        iataIdx = it->iataIndex;
    }

    if (iataIdx > 0) {
        return iata_table[iataIdx];
    }
    return  {};
}

IataCode iataCodeFromName(const QString &name)
{
    const auto fragments = name.toCaseFolded().split(QRegularExpression(QStringLiteral("[ 0-9/'\"\\(\\)&\\,.–„-]")), QString::SkipEmptyParts);
    const IataCode code = iataCodeForUniqueFragment(fragments);
    if (code.isValid()) {
        return code;
    }

    // we we didn't find a unique name fragment, try the non-unique index
    QSet<uint16_t> iataIdxs;
    for (const auto &s : fragments) {
        const auto it = std::lower_bound(nameNIndexBegin(), nameNIndexEnd(), s.toUtf8(), [](const NameNIndex &lhs, const QByteArray &rhs) {
            const auto cmp = strncmp(nameN_string_table + lhs.strOffset, rhs.constData(), std::min<int>(lhs.strLength, rhs.size()));
            if (cmp == 0) {
                return lhs.strLength < rhs.size();
            }
            return cmp < 0;
        });
        if (it == nameNIndexEnd() || it->strLength != s.toUtf8().size() || strncmp(nameN_string_table + it->strOffset, s.toUtf8().constData(), it->strLength) != 0) {
            continue;
        }

        QSet<uint16_t> candidates;
        candidates.reserve(it->iataCount);
        for (auto i = 0; i < it->iataCount; ++i) {
            candidates.insert(nameN_iata_table[it->iataOffset + i]);
        }
        if (iataIdxs.isEmpty()) { // first round
            iataIdxs = candidates;
            continue;
        }

        iataIdxs &= candidates;
        if (iataIdxs.isEmpty()) {
            break;
        }
    }

    if (iataIdxs.size() == 1) {
        return iata_table[*iataIdxs.constBegin()];
    }

    return {};
}
}
