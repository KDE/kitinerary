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

#include "timezonedb.h"
#include "timezonedb_p.h"
#include "timezonedb_data.cpp"

#include <QTimeZone>

using namespace KItinerary;

const char* KnowledgeDb::tzId(KnowledgeDb::Tz tz)
{
    return timezone_names + timezone_names_offsets[static_cast<std::underlying_type<KnowledgeDb::Tz>::type>(tz)];
}

QTimeZone KnowledgeDb::toQTimeZone(Tz tz)
{
    if (tz == Tz::Undefined) {
        return {};
    }
    return QTimeZone(tzId(tz));
}

KnowledgeDb::Tz KnowledgeDb::timezoneForCountry(CountryId country)
{
    const auto it = std::lower_bound(std::begin(country_timezone_map), std::end(country_timezone_map), country);
    if (it != std::end(country_timezone_map) && (*it).country == country) {
        return (*it).timezone;
    }

    return Tz::Undefined;
}
