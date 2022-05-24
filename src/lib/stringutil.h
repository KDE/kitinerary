/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

class QChar;
class QString;
class QStringView;

namespace KItinerary {

/** String normalization and comparison utilities. */
namespace StringUtil
{
    /** Strips out diacritics and converts to case-folded form. */
    QString normalize(QStringView str);

    /** Assuming both sides are describing the same thing, this tries to find the "better" string.
     *  That is, prefer the one that didn't lose casing/unicode/etc in previous processing.
     */
    QStringView betterString(QStringView lhs, QStringView rhs);

    /** Returns how much of the prefix of two given strings are equal, in
     *  relation to the longer of the two input strings.
     */
    float prefixSimilarity(QStringView s1, QStringView s2);

    /** Cleans up extra white spaces and XML entities from @p s. */
    QString clean(const QString &s);
}

}

