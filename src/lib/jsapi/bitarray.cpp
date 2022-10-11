/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "bitarray.h"

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
    if (m_data.size() < startBit + size || size < 0 || size > 64 || startBit < 0) {
        return 0;
    }

    return m_data.valueAtMSB<quint64>(startBit, size);
}
