/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef KITINERARY_STRINGUTIL_H
#define KITINERARY_STRINGUTIL_H

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

#endif // KITINERARY_STRINGUTIL_H
