/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

class QString;

namespace KItinerary {
namespace Generator {

/** Misc ustils for the static data code generator. */
namespace Util
{
    bool containsOnlyLetters(const QString &s);
}

}
}

