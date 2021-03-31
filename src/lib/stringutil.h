/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kitinerary_export.h"

class QChar;
class QString;

namespace KItinerary {

/** String normalization and comparison utilities. */
namespace StringUtil
{
    /** Convert @p c to case-folded form and remove diacritic marks. */
    QChar normalize(QChar c);

    /** Strips out diacritics and converts to case-folded form.
     *  @internal only exported for unit tests
     */
    KITINERARY_EXPORT QString normalize(const QString &str);
}

}

