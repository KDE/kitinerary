/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "stringutil.h"

#include <KCharsets>

#include <QDebug>
#include <QString>

#include <cstring>
#include <cctype>

using namespace KItinerary;

QString StringUtil::normalize(QStringView str)
{
    QString out;
    out.reserve(str.size());
    for (const auto c : str) {
        // case folding
        const auto n = c.toCaseFolded();

        // if the character has a canonical decomposition use that and skip the
        // combining diacritic markers following it
        // see https://en.wikipedia.org/wiki/Unicode_equivalence
        // see https://en.wikipedia.org/wiki/Combining_character
        if (n.decompositionTag() == QChar::Canonical) {
            out.push_back(n.decomposition().at(0));
        }
        // handle compatibility compositions such as ligatures
        // see https://en.wikipedia.org/wiki/Unicode_compatibility_characters
        else if (n.decompositionTag() == QChar::Compat && n.isLetter() && n.script() == QChar::Script_Latin) {
            out.append(n.decomposition());
        }
        else {
            out.push_back(n);
        }
    }
    return out;
}

static bool containsNonAscii(QStringView s)
{
    for (const auto c : s) {
        if (c.row() != 0 || c.cell() > 127) {
            return true;
        }
    }

    return false;
}

static bool isMixedCase(QStringView s)
{
    const auto letterCount = std::count_if(s.begin(), s.end(), [](auto c) { return c.isLetter(); });
    const auto upperCount = std::count_if(s.begin(), s.end(), [](auto c) { return c.isUpper(); });
    return upperCount != letterCount && upperCount != 0;
}

static int longestUpperCaseSubstring(QStringView s)
{
    int globalCount = 0;
    int count = 0;
    for (const auto c : s) {
        if (c.isUpper()) {
            ++count;
            continue;
        }
        globalCount = std::max(globalCount, count);
        count = 0;
    }
    return std::max(globalCount, count);
}

QStringView StringUtil::betterString(QStringView lhs, QStringView rhs)
{
    // prefer the one that exists at all
    if (lhs.isEmpty()) {
        return rhs;
    }
    if (rhs.isEmpty()) {
        return lhs;
    }

    // prefer Unicode over ASCII normalization
    const auto lhsNonAscii = containsNonAscii(lhs);
    const auto rhsNonAscii = containsNonAscii(rhs);
    if (lhsNonAscii && !rhsNonAscii) {
        return lhs;
    }
    if (!lhsNonAscii && rhsNonAscii) {
        return rhs;
    }

    // prefer better casing
    const auto lhsMixedCase = isMixedCase(lhs);
    const auto rhsMixedCase = isMixedCase(rhs);
    if (lhsMixedCase && !rhsMixedCase) {
        return lhs;
    }
    if (!lhsMixedCase && rhsMixedCase) {
        return rhs;
    }

    if (lhs.size() == rhs.size()) {
        if (lhsMixedCase && rhsMixedCase) {
            if (longestUpperCaseSubstring(lhs) > longestUpperCaseSubstring(rhs)) {
                return rhs;
            } else if (longestUpperCaseSubstring(lhs) < longestUpperCaseSubstring(rhs)) {
                return lhs;
            }
        }
        if (!lhsMixedCase && !rhsMixedCase) {
            if (longestUpperCaseSubstring(lhs) > longestUpperCaseSubstring(rhs)) {
                return lhs;
            }
            else if (longestUpperCaseSubstring(lhs) < longestUpperCaseSubstring(rhs)) {
                return rhs;
            }
        }
    }

    // prefer longer == more detailed version
    if (rhs.size() < lhs.size()) {
        return lhs;
    }
    return rhs;
}

float StringUtil::prefixSimilarity(QStringView s1, QStringView s2)
{
    if (s1.empty() || s2.empty()) {
        return 0.0f;
    }

    if (s1.size() > s2.size()) {
        std::swap(s1, s2);
    }

    for (int i = 0; i < s1.size(); ++i) {
        if (s1[i].toCaseFolded() == s2[i].toCaseFolded()) {
            continue;
        }
        return (float)i / (float)s2.size();
    }

    return (float)s1.size() / (float)s2.size();
}

QString StringUtil::clean(const QString &s)
{
    return KCharsets::resolveEntities(s).simplified();
}

// keep this ordered (see https://en.wikipedia.org/wiki/List_of_Unicode_characters)
struct {
    ushort key;
    const char* replacement;
} static const transliteration_map[] = {
    { u'ä', "ae" },
    { u'ö', "oe" },
    { u'ø', "oe" },
    { u'ü', "ue" },
    { u'ő', "oe" },
};

QString StringUtil::transliterate(QStringView s)
{
    QString res;
    res.reserve(s.size());

    for (const auto c : s) {
        const auto it = std::lower_bound(std::begin(transliteration_map), std::end(transliteration_map), c, [](const auto &lhs, const auto rhs) {
            return QChar(lhs.key) < rhs;
        });
        if (it != std::end(transliteration_map) && QChar((*it).key) == c) {
            res += QString::fromUtf8((*it).replacement);
            continue;
        }

        if (c.decompositionTag() == QChar::Canonical) { // see above
            res += c.decomposition().at(0);
        } else {
            res += c;
        }
    }

    return res;
}

bool StringUtil::startsWithIgnoreSpace(const QByteArray &data, const char *pattern)
{
    auto it = data.begin();
    while (it != data.end() && std::isspace(static_cast<unsigned char>(*it))) {
        ++it;
    }

    const auto len = std::strlen(pattern);
    if ((int)len >= std::distance(it, data.end())) {
        return false;
    }
    return std::strncmp(it, pattern, len) == 0;
}

QString StringUtil::simplifiedNoPlaceholder(const QString &s)
{
    if (std::all_of(s.begin(), s.end(), [](QChar c) { return c.category() == QChar::Punctuation_Dash || c.category() == QChar::Punctuation_Other; })) {
        return {};
    }
    return s.simplified();
}
