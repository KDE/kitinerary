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

#include "uic9183block.h"

using namespace KItinerary;

enum {
    BlockHeaderSize = 12,
    BlockVersionOffset = 6,
    BlockVersionSize = 2,
    BlockSizeOffset = 8,
    BlockSizeSize = 4,
};

Uic9183Block::Uic9183Block() = default;
Uic9183Block::Uic9183Block(const Uic9183Block&) = default;
Uic9183Block::Uic9183Block(Uic9183Block&&) = default;
Uic9183Block& Uic9183Block::operator=(const Uic9183Block&) = default;
Uic9183Block& Uic9183Block::operator=(Uic9183Block&&) = default;


Uic9183Block::Uic9183Block(const QByteArray &data, int offset)
    : m_data(data)
    , m_offset(offset)
{
}

// 6x header name
// 2x block version
// 4x block size as string, including the header
// followed by block payload (as 12 byte offset from header start)

const char* Uic9183Block::name() const
{
    if (isNull()) {
        return nullptr;
    }
    return m_data.constData() + m_offset;
}

const char* Uic9183Block::content() const
{
    if (isNull()) {
        return nullptr;
    }
    return m_data.constData() + m_offset + BlockHeaderSize;
}

const char* Uic9183Block::data() const
{
    if (isNull()) {
        return nullptr;
    }
    return m_data.constData() + m_offset;
}

int Uic9183Block::size() const
{
    if (m_data.size() < m_offset + BlockHeaderSize) {
        return 0;
    }
    return m_data.mid(m_offset + BlockSizeOffset, BlockSizeSize).toInt();
}

int Uic9183Block::contentSize() const
{
    return std::max(0, size() - BlockHeaderSize);
}

int Uic9183Block::version() const
{
    if (isNull()) {
        return 0;
    }
    return m_data.mid(m_offset + BlockVersionOffset, BlockVersionSize).toInt();
}

bool Uic9183Block::isNull() const
{
    return (m_data.size() < m_offset + BlockHeaderSize) || (size() > m_data.size() + m_offset);
}

Uic9183Block Uic9183Block::nextBlock() const
{
    return Uic9183Block(m_data, m_offset + size());
}
