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

    inline constexpr bool operator==(CountryId other) const
    {
        return m_id1 == other.m_id1 && m_id2 == other.m_id2;
    }

    inline constexpr bool operator!=(CountryId other) const
    {
        return m_id1 != other.m_id1 || m_id2 != other.m_id2;
    }

    inline constexpr bool isValid() const
    {
        return m_id1 != 0 || m_id2 != 0;
    }

    KITINERARY_EXPORT QString toString() const;

private:
    uint16_t m_id1 : 5;
    uint16_t m_id2 : 5;
    uint16_t m_unused : 6;
};

/** Driving side. */
enum class DrivingSide : uint8_t {
    Unknown,
    Left,
    Right,
};
Q_ENUM_NS(DrivingSide)

/** Power plug types.
 *  @note This cannot be an enum class due to QTBUG-47652.
 */
enum PowerPlugType : uint16_t {
    Unknown = 0,
    TypeA = 1 << 0,  ///< US two-pin plugs
    TypeB = 1 << 1,  ///< US three-pin plugs
    TypeC = 1 << 2,  ///< Europlug
    TypeD = 1 << 3,  ///< Type D
    TypeE = 1 << 4,  ///< French plug
    TypeF = 1 << 5,  ///< Schuko plug
    TypeG = 1 << 6,  ///< UK plug
    TypeH = 1 << 7,  ///< Israel plug
    TypeI = 1 << 8,  ///< Australian plug
    TypeJ = 1 << 9,  ///< Swiss plug
    TypeK = 1 << 10, ///< Danish plug
    TypeL = 1 << 11, ///< Type L
    TypeM = 1 << 12, ///< Type M
    TypeN = 1 << 13, ///< Type N (Brasilian)
};

Q_DECLARE_FLAGS(PowerPlugTypes, PowerPlugType)
Q_FLAG_NS(PowerPlugTypes)

/** Returns the power plugs out of @p plugs that wont fit into @p sockets. */
KITINERARY_EXPORT PowerPlugTypes incompatiblePowerPlugs(PowerPlugTypes plugs, PowerPlugTypes sockets);
/** Returns the power sockets out pf @p sockets that are unable to receive plugs
 *  out of @p plugs, excluding those in @p plugs.
 */
KITINERARY_EXPORT PowerPlugTypes incompatiblePowerSockets(PowerPlugTypes plugs, PowerPlugTypes sockets);

/** Country information. */
struct Country
{
    CountryId id;
    DrivingSide drivingSide;
    PowerPlugTypes powerPlugTypes;
    // TODO voltage/frequency
    // TODO currency
};


/** Look up contry infromation by id. */
KITINERARY_EXPORT Country countryForId(CountryId id);

/** Iterator access for the country information table. */
KITINERARY_EXPORT const Country* countriesBegin();
/** Iterator access for the country information table. */
KITINERARY_EXPORT const Country* countriesEnd();

}
}

Q_DECLARE_OPERATORS_FOR_FLAGS(KItinerary::KnowledgeDb::PowerPlugTypes)
Q_DECLARE_METATYPE(KItinerary::KnowledgeDb::DrivingSide)
Q_DECLARE_METATYPE(KItinerary::KnowledgeDb::PowerPlugTypes)

#endif // KITINERARY_COUNTRYDB_H
