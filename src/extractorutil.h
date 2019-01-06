/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

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

#ifndef KITINERARY_EXTRACTORUTIL_H
#define KITINERARY_EXTRACTORUTIL_H

#include "kitinerary_export.h"

namespace KItinerary {

class Flight;
class PostalAddress;

/** Data extraction utility functions. */
namespace ExtractorUtil
{
/** Move terminal indications from airport names to the correct fields.
 *  @internal Only exported for unit tests.
 */
KITINERARY_EXPORT Flight extractTerminals(Flight flight);

/** Try to extract postal codes included in the city name field.
 *  @internal Only exported for unit tests.
 */
KITINERARY_EXPORT PostalAddress extractPostalCode(PostalAddress addr);
}

}

#endif // KITINERARY_EXTRACTORUTIL_H
