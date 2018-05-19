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
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KITINERARY_GENERATOR_UTIL_H
#define KITINERARY_GENERATOR_UTIL_H

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

#endif // KITINERARY_GENERATOR_UTIL_H
