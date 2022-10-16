/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "bitvectorview.h"

#include <QByteArray>

using namespace KItinerary;

BitVectorView::BitVectorView() = default;

BitVectorView::BitVectorView(std::string_view data)
    : m_data(data)
{
}

BitVectorView::~BitVectorView() = default;

BitVectorView::size_type BitVectorView::size() const
{
    return m_data.size() * 8;
}

uint8_t BitVectorView::at(BitVectorView::size_type index) const
{
    const auto majIdx = index / 8;
    const auto minIdx = 7 - (index % 8);
    return (m_data.at(majIdx) & (1 << minIdx)) >> minIdx;
}

QByteArray BitVectorView::byteArrayAt(BitVectorView::size_type index, BitVectorView::size_type bytes) const
{
    QByteArray result;
    result.reserve(bytes);
    for (size_type i = 0; i < bytes; ++i) {
        const auto b = valueAtMSB<uint8_t>(index + i * 8, 8);
        result.push_back(b);
    }
    return result;
}
