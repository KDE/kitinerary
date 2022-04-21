/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

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
    QTimeZone timezoneForLocation(float lat, float lon, QStringView alpha2CountryCode, QStringView regionCode);
}
}

