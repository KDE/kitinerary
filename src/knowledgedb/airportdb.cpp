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
#include "airportdb_data.cpp"
#include "timezonedb.h"

#include <QDebug>
#include <QRegularExpression>
#include <QTimeZone>

#include <algorithm>
#include <cstring>

namespace KItinerary {
namespace AirportDb {
static_assert(sizeof(IataCode) == sizeof(uint16_t), "IATA code changed size!");
static_assert(alignof(Airport) <= sizeof(Airport), "Airport struct alignment too big!");

static constexpr auto airport_table_size = sizeof(airport_table) / sizeof(Airport);
static_assert(airport_table_size == sizeof(coordinate_table) / sizeof(KnowledgeDb::Coordinate), "Airport coordinate table size mismatch!");

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

static bool operator<(const Airport &lhs, IataCode rhs)
{
    return lhs.iataCode < rhs;
}

Coordinate coordinateForAirport(IataCode iataCode)
{
    const auto it = std::lower_bound(std::begin(airport_table), std::end(airport_table), iataCode);
    if (it == std::end(airport_table) || (*it).iataCode != iataCode) {
        return {};
    }

    return coordinate_table[std::distance(std::begin(airport_table), it)];
}

QTimeZone timezoneForAirport(IataCode iataCode)
{
    const auto it = std::lower_bound(std::begin(airport_table), std::end(airport_table), iataCode);
    if (it == std::end(airport_table) || (*it).iataCode != iataCode) {
        return {};
    }

    return (*it).timezone.toQTimeZone();
}

KnowledgeDb::CountryId countryForAirport(IataCode iataCode)
{
    const auto it = std::lower_bound(std::begin(airport_table), std::end(airport_table), iataCode);
    if (it == std::end(airport_table) || (*it).iataCode != iataCode) {
        return {};
    }

    return (*it).country;
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
        return airport_table[iataIdx].iataCode;
    }
    return {};
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
        return airport_table[*iataIdxs.constBegin()].iataCode;
    }

    return {};
}
}
}
