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

#ifndef KITINERARY_COUNTRYDB_H
#define KITINERARY_COUNTRYDB_H

#include "kitinerary_export.h"

#include <QMetaType>

#include <cstdint>

class QString;

namespace KItinerary {
namespace KnowledgeDb {
    KITINERARY_EXPORT Q_NAMESPACE

/** ISO 3166-1 alpha 2 country identifier. */
class CountryId
{
public:
    inline constexpr CountryId()
        : m_id1(0)
        , m_id2(0)
        , m_unused(0)
    {}

    inline constexpr CountryId(const char id[2])
        : m_id1(id[0] - '@')
        , m_id2(id[1] - '@')
        , m_unused(0)
    {}

    KITINERARY_EXPORT explicit CountryId(const QString &id);

    inline constexpr bool operator<(CountryId other) const
    {
        return (m_id1 << 5 | m_id2) < (other.m_id1 << 5 | other.m_id2);
    }

    inline constexpr bool operator!=(CountryId other) const
    {
        return m_id1 != other.m_id1 || m_id2 != other.m_id2;
    }

    inline constexpr bool isValid() const
    {
        return m_id1 != 0 || m_id2 != 0;
    }

private:
    uint16_t m_id1 : 5;
    uint16_t m_id2 : 5;
    uint16_t m_unused : 6;
};

enum class DrivingSide : uint8_t {
    Unknown,
    Left,
    Right,
};
Q_ENUM_NS(DrivingSide)

/** Country information. */
struct Country
{
    CountryId id;
    DrivingSide drivingSide;
    // TODO power plugs
    // TODO voltage/frequency
    // TODO currency
};


/** Look up contry infromation by id. */
KITINERARY_EXPORT Country countryForId(CountryId id);

}
}

Q_DECLARE_METATYPE(KItinerary::KnowledgeDb::DrivingSide)

#endif // KITINERARY_COUNTRYDB_H
