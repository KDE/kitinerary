/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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

quint64 BitArray::readNumberMSB(int startBit, int size) const
{
    const auto byteStart = startBit / 8;
    const auto byteCount = ((size + (startBit % 8)) / 8) + (((startBit + size) % 8) ? 1 : 0);

    if (m_data.size() < byteStart + byteCount || size < 0 || size > 64 || startBit < 0) {
        return 0;
    }

    quint64 result = 0;
    auto bitIdx = startBit % 8;
    auto outIdx = size - 1;

    for (auto byteIdx = byteStart; byteIdx < byteStart + byteCount; ++byteIdx) {
        const uint8_t byte = m_data.at(byteIdx);
        for (auto i = bitIdx; i < 8 && outIdx >= 0; ++i, --outIdx) {
            const auto bit = (byte & (1 << (7 - i))) > 0;
            if (bit) {
                result |= (1ull << outIdx);
            }
        }
        bitIdx = 0;
    }

    return result;
}
