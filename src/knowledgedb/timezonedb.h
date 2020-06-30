/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
    KITINERARY_EXPORT const char* tzId(Tz tz);

    /** Returns the corresponding QTimeZone. */
    KITINERARY_EXPORT QTimeZone toQTimeZone(Tz tz);

    /** Returns the timezone for the given country, as long as there is exactly
     *  one timezone used in that country.
     */
    KITINERARY_EXPORT Tz timezoneForCountry(CountryId country);

    /** Returns the country for a given timezone.
     *  This is unique for most IANA timezones, but not guaranteed to be so,
     *  in which case an invalid country is returned.
     */
    KITINERARY_EXPORT CountryId countryForTimezone(Tz tz);

    /** Returns the timezone for the given coordinate.
     *  The result can be @c Tz::Undefined if this cannot be clearly determined.
     */
    KITINERARY_EXPORT Tz timezoneForCoordinate(float lat, float lon, bool *ambiguous = nullptr);

    /** Returns the timezone for the given location consisting of coordinates and country.
     *  This combines the results of the two above individual queries
     *  to obtain better results close to borders.
     */
    KITINERARY_EXPORT Tz timezoneForLocation(float lat, float lon, CountryId country);

    /** Returns the country for the given coordinate.
     *  The result will be the country identifier only if the underlying
     *  spatial index has no ambiguity at the requested locations, ie. this
     *  will return less results than timezoneForCoordinate() for example.
     */
    KITINERARY_EXPORT CountryId countryForCoordinate(float lat, float lon);
}
}

#endif
