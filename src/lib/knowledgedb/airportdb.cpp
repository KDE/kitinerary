/*
   SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "airportdb.h"
#include "airportdb_p.h"
#include "airportdb_data.cpp"
#include "airportnametokenizer_p.h"
#include "stringutil.h"
#include "timezonedb_p.h"

#include <QDebug>
#include <QTimeZone>

#include <algorithm>
#include <cstring>

namespace KItinerary {
namespace KnowledgeDb {

static_assert(alignof(Airport) <= sizeof(Airport), "Airport struct alignment too big!");

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

    return (*it).coordinate;
}

QTimeZone timezoneForAirport(IataCode iataCode)
{
    const auto it = std::lower_bound(std::begin(airport_table), std::end(airport_table), iataCode);
    if (it == std::end(airport_table) || (*it).iataCode != iataCode) {
        return {};
    }

    return KnowledgeDb::timezoneForLocation((*it).coordinate.latitude, (*it).coordinate.longitude, (*it).country.toString(), {});
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
    if (res == QLatin1StringView("intl")) {
      return QStringLiteral("international");
    }

    return res;
}

static void applyTransliterations(QStringList &fragments)
{
    // note that the output has the corresponding diacritic markers already stripped,
    // as StringUtil::normalize has already been applied to fragments
    // similarly, the input is already case-folded
    for (auto &fragment : fragments) {
      fragment.replace(QLatin1StringView("ae"), QLatin1StringView("a"));
      fragment.replace(QLatin1StringView("oe"), QLatin1StringView("o"));
      fragment.replace(QLatin1StringView("ue"), QLatin1StringView("u"));
    }
}

// HACK to work around MSVC string length limit
static const char* name1_string_table(uint32_t offset)
{
    if (offset < sizeof(name1_string_table_0)) {
        return name1_string_table_0 + offset;
    }
    return name1_string_table_1 + (offset - sizeof(name1_string_table_0)) + 1; // +1 to compensate for the trailing null byte in name1_string_table_0
}

static IataCode iataCodeForUniqueFragment(const QString &s)
{
    const auto it = std::lower_bound(std::begin(name1_string_index), std::end(name1_string_index), s.toUtf8(), [](const Name1Index &lhs, const QByteArray &rhs) {
        const auto cmp = strncmp(name1_string_table(lhs.offset()), rhs.constData(), std::min<int>(lhs.length, rhs.size()));
        if (cmp == 0) {
            return lhs.length < rhs.size();
        }
        return cmp < 0;
    });
    if (it == std::end(name1_string_index) || it->length != s.toUtf8().size() || strncmp(name1_string_table(it->offset()), s.toUtf8().constData(), it->length) != 0) {
        return {};
    }
    return airport_table[it->iataIndex].iataCode;
}

static void iataCodeForUniqueFragment(const QStringList &fragments, std::vector<IataCode> &codes)
{
    for (const auto &s : fragments) {
        const auto foundCode = iataCodeForUniqueFragment(s);
        if (!foundCode.isValid()) {
            continue;
        }

        auto it = std::lower_bound(codes.begin(), codes.end(), foundCode);
        if (it == codes.end() || (*it) != foundCode) {
            codes.insert(it, foundCode);
        }
    }
}

static void iataCodeForNonUniqueFragments(const QStringList &fragments, std::vector<IataCode> &codes)
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

        // TODO we can do this in-place in codes
        QSet<uint16_t> candidates;
        candidates.reserve(it->iataCount);
        for (auto i = 0; i < it->iataCount; ++i) {
            candidates.insert(nameN_iata_table[it->iataOffset + i]);
        }
        if (iataIdxs.isEmpty()) { // first round
            iataIdxs = candidates;
            continue;
        }

        // ignore the imprecisely used "international" if it results in an empty set here
        if (s == QLatin1StringView("international") &&
            !iataIdxs.intersects(candidates)) {
          continue;
        }

        iataIdxs &= candidates;
        if (iataIdxs.isEmpty()) {
            break;
        }
    }

    std::transform(iataIdxs.begin(), iataIdxs.end(), std::back_inserter(codes), [](const auto idx) { return airport_table[idx].iataCode; });
    std::sort(codes.begin(), codes.end());
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

static void iataCodeForNameFragments(const QStringList &fragments, std::vector<IataCode> &codes)
{
    iataCodeForUniqueFragment(fragments, codes);
    if (!codes.empty()) {
        return;
    }
    iataCodeForNonUniqueFragments(fragments, codes);
}

static QStringList splitToFragments(QStringView name)
{
    AirportNameTokenizer tokenizer(name);
    return tokenizer.toStringList();
}

}

std::vector<KnowledgeDb::IataCode> KnowledgeDb::iataCodesFromName(QStringView name)
{
    const auto fragments = splitToFragments(name);
    QStringList normalizedFragments;
    normalizedFragments.reserve(fragments.size());
    std::transform(fragments.begin(), fragments.end(), std::back_inserter(normalizedFragments), [](const auto &s) { return normalizeFragment(s); });

    std::vector<IataCode> codes;
    std::vector<IataCode> candidates;
    iataCodeForNameFragments(normalizedFragments, codes);

    // try again, with alternative translitarations of e.g. umlauts replaced
    applyTransliterations(normalizedFragments);
    iataCodeForNameFragments(normalizedFragments, candidates);
    if (!candidates.empty() && (codes.empty() || candidates.size() < codes.size())) {
        codes = std::move(candidates);
    }

    // check if the name contained the IATA code as disambiguation already
    const auto code = iataCodeForIataCodeFragment(fragments);
    if (code.isValid() && std::find(codes.begin(), codes.end(), code) != codes.end()) {
        return {code};
    }

    // attempt to cut off possibly confusing fancy terminal names
    auto it = std::find(normalizedFragments.begin(), normalizedFragments.end(), QStringLiteral("terminal"));
    if (it != normalizedFragments.end()) {
        normalizedFragments.erase(it, normalizedFragments.end());
        candidates.clear();
        iataCodeForNameFragments(normalizedFragments, candidates);
        if (!candidates.empty() && (codes.empty() || candidates.size() < codes.size())) {
            codes = std::move(candidates);
        }
    }
    return codes;
}

}
