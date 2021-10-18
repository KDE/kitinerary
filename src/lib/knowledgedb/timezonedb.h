/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <kitinerary_export.h>

#include <cstdint>
#include <limits>

class QString;
class QStringView;
class QTimeZone;

namespace KItinerary {
namespace KnowledgeDb {
    /** Returns the timezone for the given location consisting of coordinates and/or country.
     *  Either argument can be omitted, if both are available better results can be provided.
     */
    KITINERARY_EXPORT QTimeZone timezoneForLocation(float lat, float lon, QStringView alpha2CountryCode);

    /** Returns the country for the given coordinate.
     *  The result will be the country identifier only if the underlying
     *  spatial index has no ambiguity at the requested locations, ie. this
     *  will return less results than timezoneForCoordinate() for example.
     */
    KITINERARY_EXPORT QString countryForCoordinate(float lat, float lon);
}
}

