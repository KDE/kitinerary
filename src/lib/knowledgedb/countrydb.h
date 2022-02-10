/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kitinerary_export.h"
#include "alphaid.h"

#include <QMetaType>

#include <cstdint>


namespace KItinerary {
namespace KnowledgeDb {
    KITINERARY_EXPORT Q_NAMESPACE

/** ISO 3166-1 alpha 2 country identifier. */
using CountryId = AlphaId<uint16_t, 2>;

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
Q_DECLARE_OPERATORS_FOR_FLAGS(PowerPlugTypes)
Q_FLAG_NS(PowerPlugTypes)

/** Returns the power plugs out of @p plugs that won't fit into @p sockets. */
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
};


/** Look up country information by id. */
KITINERARY_EXPORT Country countryForId(CountryId id);

/** Look up country ISO code from a UIC country code. */
KITINERARY_EXPORT CountryId countryIdForUicCode(uint16_t uicCountryCode);

}
}

Q_DECLARE_METATYPE(KItinerary::KnowledgeDb::DrivingSide)
Q_DECLARE_METATYPE(KItinerary::KnowledgeDb::PowerPlugTypes)

