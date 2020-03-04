/*
    Copyright (C) 2020 Volker Krause <vkrause@kde.org>

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

#include "berelement_p.h"

#include <QDebug>

using namespace KItinerary;

enum {
    BerLongTypeMask = 0x1F,
    BerExtendedTypeMask = 0x80,
    BerExtendedLenghtMask = 0x80,
    BerVariableLengthMarker = 0x80,
};

BER::Element::Element() = default;

BER::Element::Element(const QByteArray &data, int offset, int size)
    : m_data(data)
    , m_offset(offset)
    , m_dataSize(size < 0 ? data.size() : std::min(offset + size, data.size()))
{
    assert(m_dataSize <= m_data.size());
    if (!isValid()) {
        m_data.clear();
        m_offset = -1;
        m_dataSize = -1;
    }
}

BER::Element::~Element() = default;

bool BER::Element::isValid() const
{
    if (m_offset < 0 || m_dataSize <= 0 || m_offset + 2 > m_dataSize) {
        return false;
    }

    // check type size
    const auto ts = typeSize();
    if (ts < 0 || ts >= 4 || m_offset + ts + 1 > m_dataSize) {
        return false;
    }

    // check size of length field
    const auto ls = lengthSize();
    if (ls <= 0 || ls >= 4 || m_offset + ts + ls > m_dataSize) {
        return false;
    }

    // check size of the content
    const auto cs = contentSize();
    return cs >= 0 && m_offset + ts + ls + cs <= m_dataSize;
}

int BER::Element::typeSize() const
{
    assert(m_offset >= 0);
    assert(m_offset + 1 < m_dataSize);
    auto it = m_data.begin() + m_offset;
    if (((*it) & BerLongTypeMask) != BerLongTypeMask) {
        return 1;
    }

    while (it != m_data.end() && std::distance(m_data.begin(), it) < m_dataSize) {
        ++it;
        if (((*it) & BerExtendedTypeMask) == 0) {
            return std::distance(m_data.begin(), it) - m_offset + 1;
        }
    }

    return 0;
}

uint32_t BER::Element::type() const
{
    const auto ts = typeSize();

    // this is actually complete wrong compared to BER encoding rules, but since pretty much all docs
    // we works with use the encoded type ids rather than the decoded ones this is actually much more practical
    // might be worth to eventually have two methods for this, one with proper decoding and one with the raw data
    uint32_t result = 0;
    for (int i = 0; i < ts; ++i) {
        result <<= 8;
        result += (uint8_t)*(m_data.constData() + m_offset + i);
    }
    return result;
}

int BER::Element::lengthSize() const
{
    const auto ts = typeSize();
    const uint8_t firstLengthByte = *(m_data.constData() + m_offset + ts);
    if (firstLengthByte == BerVariableLengthMarker) {
        return 1;
    }
    if (firstLengthByte & BerExtendedLenghtMask) {
        return (firstLengthByte & ~BerExtendedLenghtMask) + 1;
    }
    return 1;
}

int BER::Element::size() const
{
    const auto ts = typeSize();
    const auto s = ts + lengthSize() + contentSize();
    const uint8_t firstLengthByte = *(m_data.constData() + m_offset + ts);
    if (firstLengthByte == BerVariableLengthMarker) {
        return s + 2;
    }
    return s;
}

int BER::Element::contentSize() const
{
    const auto ts = typeSize();
    const uint8_t firstLengthByte = *(m_data.constData() + m_offset + ts);
    if (firstLengthByte == BerVariableLengthMarker) {
        const auto idx = m_data.indexOf(QByteArray("\0\0", 2), m_offset + ts + 1);
        if (idx + 1 > m_dataSize) {
            return - 1;
        }
        return idx - m_offset - ts - 1;
    }
    if (firstLengthByte & BerExtendedLenghtMask) {
        const auto ls = firstLengthByte & ~BerExtendedLenghtMask;
        int result = 0;
        for (int i = 0; i < ls; ++i) {
            result <<= 8;
            result += (uint8_t)*(m_data.constData() + m_offset + ts + 1 + i);
        }
        return result;
    }
    return firstLengthByte;
}

int BER::Element::contentOffset() const
{
    return m_offset + typeSize() + lengthSize();
}

const uint8_t* BER::Element::contentData() const
{
    return reinterpret_cast<const uint8_t*>(m_data.constData() + contentOffset());
}

BER::Element BER::Element::first() const
{
    return BER::Element(m_data, contentOffset(), contentSize());
}

BER::Element BER::Element::next() const
{
    const auto s = size();
    if (m_dataSize <= m_offset + s) {
        return {};
    }
    return BER::Element(m_data, m_offset + s, m_dataSize - m_offset - s);
}

BER::Element BER::Element::find(uint32_t type) const
{
    auto e = first();
    while (e.isValid()) {
        if (e.type() == type) {
            return e;
        }
        e = e.next();
    }
    return {};
}
