/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "util.h"

#include <QString>

using namespace KItinerary::Generator;

bool Util::containsOnlyLetters(const QString& s)
{
    for (const auto c : s) {
        if (c < QLatin1Char('A') || c > QLatin1Char('Z')) {
            return false;
        }
    }
    return true;
}
