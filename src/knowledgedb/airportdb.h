/*
   Copyright (c) 2017 Volker Krause <vkrause@kde.org>

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   This library is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
   License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.
*/

#ifndef AIRPORTDB_H
#define AIRPORTDB_H

#include "kitinerary_export.h"
#include "countrydb.h"
#include "iatacode.h"
#include "knowledgedb.h"
#include "timezonedb.h"

class QString;
class QTimeZone;

namespace KItinerary {
namespace KnowledgeDb {

/** Airport information structure as used in the database.
 *  @internal
 */
struct Airport {
    IataCode iataCode;
    CountryId country;
    Timezone timezone;
};

/** Returns the geographical coordinates the airport with IATA code @p iataCode is in. */
KITINERARY_EXPORT Coordinate coordinateForAirport(IataCode iataCode);

/** Returns the timezone the airport with IATA code @p iataCode is in. */
KITINERARY_EXPORT QTimeZone timezoneForAirport(IataCode iataCode);

/** Returns the country the airport with IATA code @p iataCode is in. */
KITINERARY_EXPORT CountryId countryForAirport(IataCode iataCode);

/** Attempts to find the unique IATA code for the given airport name. */
KITINERARY_EXPORT IataCode iataCodeFromName(const QString &name);
/** Returns all possible IATA code candidates for the given airport name. */
KITINERARY_EXPORT std::vector<IataCode> iataCodesFromName(const QString &name);
}

}

#endif // AIRPORTDB_H
