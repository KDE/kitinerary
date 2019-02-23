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
#include "stringutil.h"
#include "timezonedb.h"

#include <QDebug>
#include <QRegularExpression>
#include <QTimeZone>

#include <algorithm>
#include <cstring>

namespace KItinerary {
namespace KnowledgeDb {
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

static QString normalizeFragment(const QString &s)
{
    auto res = StringUtil::normalize(s);
    // resolve abbreviations
    if (res == QLatin1String("intl")) return QStringLiteral("international");

    return res;
}

static void applyTransliterations(QStringList &fragments)
{
    // note that the output has the corresponding diacritic markers already stripped,
    // as StringUtil::normalize has already been applied to fragments
    // similarly, the input is already case-folded
    for (auto &fragment : fragments) {
        fragment.replace(QLatin1String("ae"), QLatin1String("a"));
        fragment.replace(QLatin1String("oe"), QLatin1String("o"));
        fragment.replace(QLatin1String("ue"), QLatin1String("u"));
    }
}

static IataCode iataCodeForUniqueFragment(const QString &s)
{
    const auto it = std::lower_bound(std::begin(name1_string_index), std::end(name1_string_index), s.toUtf8(), [](const Name1Index &lhs, const QByteArray &rhs) {
        const auto cmp = strncmp(name1_string_table + lhs.offset(), rhs.constData(), std::min<int>(lhs.length, rhs.size()));
        if (cmp == 0) {
            return lhs.length < rhs.size();
        }
        return cmp < 0;
    });
    if (it == std::end(name1_string_index) || it->length != s.toUtf8().size() || strncmp(name1_string_table + it->offset(), s.toUtf8().constData(), it->length) != 0) {
        return {};
    }
    return airport_table[it->iataIndex].iataCode;
}

static IataCode iataCodeForUniqueFragment(const QStringList &fragments)
{
    IataCode resultCode;
    for (const auto &s : fragments) {
        const auto foundCode = iataCodeForUniqueFragment(s);
        if (!foundCode.isValid()) {
            continue;
        }

        if (resultCode.isValid() && resultCode != foundCode) {
            return {}; // not unique
        }
        resultCode = foundCode;
    }

    return resultCode;
}

static IataCode iataCodeForNonUniqueFragments(const QStringList &fragments)
{
    // we didn't find a unique name fragment, try the non-unique index
    QSet<uint16_t> iataIdxs;
    for (const auto &s : fragments) {
        const auto it = std::lower_bound(std::begin(nameN_string_index), std::end(nameN_string_index), s.toUtf8(), [](const NameNIndex &lhs, const QByteArray &rhs) {
                const auto cmp = strncmp(nameN_string_table + lhs.strOffset, rhs.constData(), std::min<int>(lhs.strLength, rhs.size()));
                if (cmp == 0) {
                    return lhs.strLength < rhs.size();
                }
                return cmp < 0;
            });
        if (it == std::end(nameN_string_index) || it->strLength != s.toUtf8().size() || strncmp(nameN_string_table + it->strOffset, s.toUtf8().constData(), it->strLength) != 0) {
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

static IataCode iataCodeForIataCodeFragment(const QStringList &fragments)
{
    IataCode code;
    for (const auto &s : fragments) {
        if (s.size() != 3) {
            continue;
        }
        if (!std::all_of(s.begin(), s.end(), [](const auto c) { return c.isUpper(); })) {
            continue;
        }
        const IataCode searchCode{s};
        if (code.isValid() && searchCode != code) {
            return {};
        }
        const auto it = std::lower_bound(std::begin(airport_table), std::end(airport_table), searchCode);
        if (it != std::end(airport_table) && (*it).iataCode == searchCode) {
            code = searchCode;
        }
        // check that this is only a IATA code, not also a (conflicting) name fragment
        const auto uniqueFragmentCode = iataCodeForUniqueFragment(normalizeFragment(s));
        if (uniqueFragmentCode.isValid() && code.isValid() && uniqueFragmentCode != code) {
            return {};
        }
    }
    return code;
}

static IataCode iataCodeForNameFragments(const QStringList &fragments)
{
    IataCode code = iataCodeForUniqueFragment(fragments);
    if (code.isValid()) {
        return code;
    }
    return iataCodeForNonUniqueFragments(fragments);
}

IataCode iataCodeFromName(const QString &name)
{
    const auto fragments = name.split(QRegularExpression(QStringLiteral("[ 0-9/'\"\\(\\)&\\,.–„-]")), QString::SkipEmptyParts);
    QStringList normalizedFragments;
    normalizedFragments.reserve(fragments.size());
    std::transform(fragments.begin(), fragments.end(), std::back_inserter(normalizedFragments), [](const auto &s) { return normalizeFragment(s); });

    IataCode code = iataCodeForNameFragments(normalizedFragments);
    if (code.isValid()) {
        return code;
    }

    // try again, with alternative translitarations of e.g. umlauts replaced
    applyTransliterations(normalizedFragments);
    code = iataCodeForNameFragments(normalizedFragments);
    if (code.isValid()) {
        return code;
    }

    // check if the name contained the IATA code as disambiguation already
    return iataCodeForIataCodeFragment(fragments);
}
}
}
