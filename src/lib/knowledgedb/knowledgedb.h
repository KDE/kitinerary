/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <cmath>
#include <cstdint>

namespace KItinerary {

/** Lookup functions, utilities and data types for the static knowledge database.
 *  The content accessible by this functions is extracted from Wikidata and compiled
 *  into this library.
 *  @note The types in this namespace match the binary storage structure and thus
 *  are not intended for use in binary compatible APIs.
 */
namespace KnowledgeDb
{

/** Geographical coordinate.
 *  This matches the binary data layout on disk, it's not intended
 *  for use in API.
 */
struct Coordinate {
    inline constexpr Coordinate()
        : longitude(NAN)
        , latitude(NAN)
    {
    }

    inline explicit constexpr Coordinate(float lng, float lat)
        : longitude(lng)
        , latitude(lat)
    {
    }

    inline bool isValid() const
    {
        return !std::isnan(latitude) && !std::isnan(longitude);
    }

    inline constexpr bool operator==(Coordinate other) const
    {
        return latitude == other.latitude && longitude == other.longitude;
    }

    float longitude;
    float latitude;
};

/** Unalinged storage of a numerical value.
 *  This is optimized for a compact memory layout, at the expense of slightly more
 *  expensive comparison operations.
 *  @tparam N the size in byte, at this point limited to at most 4
 */
template <int N> class UnalignedNumber : private UnalignedNumber<N-1> {
public:
    inline constexpr UnalignedNumber() = default;
    inline explicit constexpr UnalignedNumber(uint32_t num)
        : UnalignedNumber<N-1>(num)
        , m_value((num & (0xFF << (N-1)*8)) >> (N-1)*8)
    {}

    inline constexpr bool operator==(UnalignedNumber<N> other) const
    {
        if (m_value == other.m_value) {
            return UnalignedNumber<N-1>::operator==(other);
        }
        return false;
    }
    inline constexpr bool operator!=(UnalignedNumber<N> other) const
    {
        if (m_value == other.m_value) {
            return UnalignedNumber<N-1>::operator!=(other);
        }
        return true;
    }
    inline constexpr bool operator<(UnalignedNumber<N> other) const
    {
        if (m_value == other.m_value) {
            return UnalignedNumber<N-1>::operator<(other);
        }
        return m_value < other.m_value;
    }

    inline constexpr UnalignedNumber<N>& operator=(uint32_t num)
    {
        setValue(num);
        return *this;
    }
    inline constexpr UnalignedNumber<N>& operator|=(uint32_t num)
    {
        setValue(value() | num);
        return *this;
    }

    inline constexpr operator uint32_t() const
    {
        return value();
    }

    inline constexpr uint32_t value() const
    {
        return UnalignedNumber<N-1>::value() | (m_value << (N-1)*8);
    }

protected:
    inline constexpr void setValue(uint32_t num)
    {
        m_value = (num & (0xFF << (N-1)*8)) >> (N-1)*8;
        UnalignedNumber<N-1>::setValue(num);
    }

private:
    uint8_t m_value = 0;
};

template <> class UnalignedNumber<1> {
public:
    inline constexpr UnalignedNumber() = default;
    inline explicit constexpr UnalignedNumber(uint32_t num)
        : m_value(num & 0xFF)
    {}

    inline constexpr bool operator==(UnalignedNumber<1> other) const
    {
        return m_value == other.m_value;
    }
    inline constexpr bool operator!=(UnalignedNumber<1> other) const
    {
        return m_value != other.m_value;
    }
    inline constexpr bool operator<(UnalignedNumber<1> other) const
    {
        return m_value < other.m_value;
    }

    inline constexpr uint32_t value() const
    {
        return m_value;
    }

protected:
    inline constexpr void setValue(uint32_t num)
    {
        m_value = num & 0xFF;
    }

private:
    uint8_t m_value = 0;
};

}

}

