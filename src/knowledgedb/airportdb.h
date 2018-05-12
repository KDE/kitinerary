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
#include "knowledgedb.h"

#include <cmath>
#include <cstdint>

class QString;
class QTimeZone;

namespace KItinerary {

/** Database of all civilian airports, their locations and timezones. */
namespace AirportDb {

/** IATA airport code. */
class KITINERARY_EXPORT IataCode
{
public:
    inline constexpr IataCode()
        : m_letter0(0)
        , m_letter1(0)
        , m_letter2(0)
        , m_valid(0)
    {
    }

    inline explicit constexpr IataCode(const char iataStr[4])
        : m_letter0(iataStr[0] - 'A')
        , m_letter1(iataStr[1] - 'A')
        , m_letter2(iataStr[2] - 'A')
        , m_valid(1)
    {
    }

    explicit IataCode(const QString &iataStr);
    bool isValid() const;
    bool operator<(IataCode rhs) const;
    bool operator==(IataCode other) const;
    bool operator!=(IataCode other) const;

    QString toString() const;
private:
    uint16_t toUInt16() const;
    uint16_t m_letter0 : 5;
    uint16_t m_letter1 : 5;
    uint16_t m_letter2 : 5;
    uint16_t m_valid : 1;
};

/** Returns the geographical coordinates the airport with IATA code @p iataCode is in. */
KITINERARY_EXPORT KnowledgeDb::Coordinate coordinateForAirport(IataCode iataCode);

/** Returns the timezone the airport with IATA code @p iataCode is in. */
KITINERARY_EXPORT QTimeZone timezoneForAirport(IataCode iataCode);

/** Attempts to find the unique IATA code for the given airport name. */
KITINERARY_EXPORT IataCode iataCodeFromName(const QString &name);
}

}

#endif // AIRPORTDB_H
