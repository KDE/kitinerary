/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "stringutil.h"

#include <QDebug>
#include <QString>

using namespace KItinerary;

QChar StringUtil::normalize(QChar c)
{
    // case folding
    const auto n = c.toCaseFolded();

    // if the character has a canonical decomposition use that and skip the
    // combining diacritic markers following it
    // see https://en.wikipedia.org/wiki/Unicode_equivalence
    // see https://en.wikipedia.org/wiki/Combining_character
    if (n.decompositionTag() == QChar::Canonical) {
        return n.decomposition().at(0);
    }

    return n;
}

QString StringUtil::normalize(const QString &str)
{
    QString out;
    out.reserve(str.size());
    for (const auto c : str) {
        out.push_back(normalize(c));
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
    const auto upperCount = std::count_if(s.begin(), s.end(), [](auto c) { return c.isUpper(); });
    return upperCount != s.size() && upperCount != 0;
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

    // prefer longer == more detailed version
    if (rhs.size() < lhs.size()) {
        return lhs;
    }
    return rhs;
}
