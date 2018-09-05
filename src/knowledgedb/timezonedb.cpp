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

#include "timezonedb.h"
#include "timezonedb_p.h"
#include "timezonedb_data.cpp"

#include <QTimeZone>

using namespace KItinerary::KnowledgeDb;

QTimeZone Timezone::toQTimeZone() const
{
    if (offset > sizeof(timezone_names)) {
        return {};
    }
    return QTimeZone(timezone_names + offset);
}

/* Manual overrides for countries that de-facto only have a single timezone,
 * even if the IANA database doesn't reflect that.
 *
 * Must be sorted by CountryId!
 */
static constexpr const CountryTimezoneMap country_timezone_overrides[] = {
    {CountryId{"CN"}, Tz::Asia_Shanghai },
    {CountryId{"CY"}, Tz::Asia_Nicosia },
    {CountryId{"DE"}, Tz::Europe_Berlin },
    {CountryId{"MY"}, Tz::Asia_Kuala_Lumpur },
};

Timezone KItinerary::KnowledgeDb::timezoneForCountry(CountryId country)
{
    auto it = std::lower_bound(std::begin(country_timezone_overrides), std::end(country_timezone_overrides), country);
    if (it != std::end(country_timezone_overrides) && (*it).country == country) {
        return (*it).timezone;
    }

    it = std::lower_bound(std::begin(country_timezone_map), std::end(country_timezone_map), country);
    if (it != std::end(country_timezone_map) && (*it).country == country) {
        return (*it).timezone;
    }

    return {};
}
