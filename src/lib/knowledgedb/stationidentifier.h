/*
    SPDX-FileCopyrightText: 2018-2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kitinerary_export.h"

#include "alphaid.h"
#include "knowledgedb.h"

class QString;

namespace KItinerary {
namespace KnowledgeDb {

/** Base class for UIC/IBNR station identifiers. */
class UICIdentiferBase : public UnalignedNumber<3> {
public:
    inline explicit constexpr UICIdentiferBase() = default;
    inline explicit constexpr UICIdentiferBase(uint32_t id) :
        UnalignedNumber<3>(id > 9999999 ? id / 10 : id) // strip off check digit if present
    {}

    KITINERARY_EXPORT UICIdentiferBase(const QString &id);

    inline constexpr bool isValid() const
    {
        return value() >= 1000000 && value() <= 9999999;
    }
};

/** IBNR station id.
 *  2 digits UIC country code, 5 digits station id.
 *  Same format as UICStation, but nevertheless different values.
 */
class IBNR : public UICIdentiferBase {
    using UICIdentiferBase::UICIdentiferBase;
};

/** UIC station id.
 *  2 digits UIC country code, 5 digits station id.
 *  Same format as IBNR, but nevertheless different values.
 */
class UICStation : public UICIdentiferBase {
    using UICIdentiferBase::UICIdentiferBase;
};


/** Base class for SNCF/Benerail station identifiers. */
class FiveAlphaId : public UnalignedNumber<3> {
public:
    inline explicit constexpr FiveAlphaId() = default;
    inline explicit constexpr FiveAlphaId(const char s[5])
        : UnalignedNumber<3>(fromChars(s))
    {
    }

    KITINERARY_EXPORT explicit FiveAlphaId(const QString &id);

    inline constexpr bool isValid() const
    {
        return value() > 0;
    }

    KITINERARY_EXPORT QString toString() const;

private:
    static inline constexpr uint32_t fromChars(const char s[5])
    {
        return (s[4] - '@') + 27 * ((s[3] - '@') + 27 * ((s[2] - '@') + 27 * ((s[1] - '@') + 27 * (s[0] - '@'))));
    }
};

/** SNCF station id.
 *  2 letters ISO country code, 3 letters station id, expected to be in upper case.
 */
class SncfStationId : public FiveAlphaId
{
    using FiveAlphaId::FiveAlphaId;
};

/** Benerail station id.
 *  2 letters ISO country code, 3 letters station id, expected to be in upper case.
 */
class BenerailStationId : public FiveAlphaId
{
    using FiveAlphaId::FiveAlphaId;
};


/** VR (Finland) station codes.
 *  2 to 4 letter uppercase alphabetic code.
 */
class VRStationCode : public UnalignedNumber<3>
{
public:
    inline constexpr VRStationCode() = default;
    inline explicit constexpr VRStationCode(const char s[4])
        : UnalignedNumber<3>(fromChars(s))
    {}
    KITINERARY_EXPORT explicit VRStationCode(const QString &id);

    [[nodiscard]] inline constexpr bool isValid() const
    {
        return value() > 0;
    }

    [[nodiscard]] KITINERARY_EXPORT QString toString() const;

private:
    [[nodiscard]] static inline constexpr uint32_t charVal(uint8_t c)
    {
        switch (c) {
            case '\0': return 0;
            case 0xC4: return 27; // Ä in Latin-1
            case 0xD6: return 28; // Ö in Latin-1
        }
        return c - '@';
    }
    [[nodiscard]] static inline constexpr uint32_t fromChars(const char s[4])
    {
        return (charVal(s[0]) << 18) + (charVal(s[1]) << 12) + (charVal(s[2]) << 6) + charVal(s[3]);
    }
};

/** Amtrak staion codes. */
using AmtrakStationCode = AlphaId<uint16_t, 3>;
/** Via Rail station code. */
using ViaRailStationCode = AlphaId<UnalignedNumber<3>, 4>;
/** UK railway station code. */
using UKRailwayStationCode = AlphaId<uint16_t, 3>;
}
}
