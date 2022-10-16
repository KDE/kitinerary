/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "bitarray.h"

#include "asn1/bitvectorview.h"

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
    BitVectorView view(std::string_view(m_data.constData(), m_data.size()));
    if ((int)view.size() < startBit + size || size < 0 || size > 64 || startBit < 0) {
        return 0;
    }

    return view.valueAtMSB<quint64>(startBit, size);
}
