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

#ifndef AIRPORTDB_P_H
#define AIRPORTDB_P_H

#include <cstdint>

namespace AirportDb  {
// pack 24 bit offset, 8 bit length and 16 bit IATA index into 48bit with 16bit alignment
struct NameIndex
{
    inline explicit constexpr NameIndex(uint32_t offset, uint8_t len, uint16_t idx)
        : offset1((offset & 0x00ffff00) >> 8)
        , offset2(offset & 0x000000ff)
        , length(len)
        , iataIndex(idx)
    {
    }

    inline constexpr uint32_t offset() const
    {
        return offset1 << 8 | offset2;
    }

    uint16_t offset1;
    uint8_t offset2;
    uint8_t length;
    uint16_t iataIndex;
};

static_assert(sizeof(NameIndex) == 6, "NameIndex size changed!");
static_assert(alignof(NameIndex) <= 2, "NameIndex alignment changed!");
}

#endif
