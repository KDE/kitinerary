/*
   SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

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
    Coordinate coordinate;
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

