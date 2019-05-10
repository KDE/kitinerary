/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

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

#include "bitarray.h"

#include <QByteArray>
#include <QDebug>

#include <cstdint>

using namespace KItinerary::JsApi;

BitArray::BitArray() = default;
BitArray::BitArray(const QByteArray &data)
    : m_data(data)
{
}

BitArray::~BitArray() = default;

int BitArray::readNumberMSB(int startBit, int size) const
{
    if (m_data.size() <= ((startBit + size) / 8) || size < 0 || size > 32 || startBit < 0) {
        return 0;
    }

    int result = 0;

    const auto byteStart = startBit / 8;
    const auto byteCount = ((size + (startBit % 8)) / 8) + (((startBit + size) % 8) ? 1 : 0);
    auto bitIdx = startBit % 8;
    auto outIdx = size - 1;

    for (auto byteIdx = byteStart; byteIdx < byteStart + byteCount; ++byteIdx) {
        const uint8_t byte = m_data.at(byteIdx);
        for (auto i = bitIdx; i < 8 && outIdx >= 0; ++i, --outIdx) {
            const auto bit = (byte & (1 << (7 - i))) > 0;
            if (bit) {
                result |= (1 << outIdx);
            }
        }
        bitIdx = 0;
    }

    return result;
}
