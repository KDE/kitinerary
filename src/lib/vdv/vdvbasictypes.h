/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_VDVBASICTYPES_H
#define KITINERARY_VDVBASICTYPES_H

#include <QDateTime>

#include <cstdint>

/** @file vdvbasictypes.h
 *  Low-level data types used in VDV ticket structs.
 */

namespace KItinerary {

#pragma pack(push)
#pragma pack(1)

/** Two-digit BCD encoded number. */
template <int N>
struct VdvBcdNumber
{
    static_assert(N > 0 && N <= 4);
    uint8_t data[N];

    inline constexpr uint32_t value() const
    {
        uint32_t v = 0;
        for (int i = 0; i < N; ++i) {
            v *= 100;
            v += ((data[i] & 0xF0) >> 4) * 10 + (data[i] & 0x0F);
        }
        return v;
    }

    inline constexpr operator uint32_t() const { return value(); }
};

/** Date encoded as 8 BCD digits. */
struct VdvBcdDate
{
    VdvBcdNumber<2> bcdYear;
    VdvBcdNumber<1> bcdMonth;
    VdvBcdNumber<1> bcdDay;

    inline QDate value() const
    {
        return QDate(bcdYear, bcdMonth, bcdDay);
    }

    inline operator QDate() const { return value(); }
};

/** Big-endian numeric value. */
template <int N>
struct VdvNumber
{
    static_assert(N > 0 && N <= 4);
    uint8_t data[N];

    inline constexpr uint32_t value() const
    {
        uint32_t v = 0;
        for (int i = 0; i < N; ++i) {
            v <<= 8;
            v |= data[i];
        }
        return v;
    }

    inline constexpr operator uint32_t() const { return value(); }
};

/** Date/time representation encoded in 4 byte. */
struct VdvDateTimeCompact
{
    VdvNumber<4> data;

    inline QDateTime value() const
    {
        return QDateTime(
            {
                (int)((data & 0b1111'1110'0000'0000'0000'0000'0000'0000) >> 25) + 1990,
                (int)(data & 0b0000'0001'1110'0000'0000'0000'0000'0000) >> 21,
                (int)(data & 0b0000'0000'0001'1111'0000'0000'0000'0000) >> 16
            }, {
                (int)(data & 0b0000'0000'0000'0000'1111'1000'0000'0000) >> 11,
                (int)(data & 0b0000'0000'0000'0000'0000'0111'1110'0000) >> 5,
                (int)(data & 0b0000'0000'0000'0000'0000'0000'0001'1111) * 2
            });
    }

    inline operator QDateTime() const { return value(); }
};

#pragma pack(pop)

}

#endif // KITINERARY_VDVBASICTYPES_H
