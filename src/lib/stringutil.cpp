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
