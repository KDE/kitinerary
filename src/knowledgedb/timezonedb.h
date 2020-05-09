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

#ifndef KITINERARY_TIMEZONEDB_H
#define KITINERARY_TIMEZONEDB_H

#include <kitinerary_export.h>
#include "countrydb.h"
#include "timezonedb_data.h"

#include <cstdint>
#include <limits>

class QTimeZone;

namespace KItinerary {
namespace KnowledgeDb {
    /** Returns the IANA timezone id for @p tz. */
    const char* tzId(Tz tz);

    /** Returns the corresponding QTimeZone. */
    KITINERARY_EXPORT QTimeZone toQTimeZone(Tz tz);

    /** Returns the timezone for the given country, as long as there is exactly
     *  one timezone used in that country.
     */
    KITINERARY_EXPORT Tz timezoneForCountry(CountryId country);
}
}

#endif
