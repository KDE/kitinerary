/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "uic9183block.h"
#include "uic9183utils.h"
#include "logging.h"

#include <cstring>

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
    : m_offset(offset)
{
    if (data.size() < offset + BlockHeaderSize) {
        return;
    }

    const auto blockSize = Uic9183Utils::readAsciiEncodedNumber(data, offset + BlockSizeOffset, BlockSizeSize);
    if (data.size() < (blockSize + offset) || blockSize < BlockHeaderSize) {
        return;
    }

    m_data = data;
}

bool Uic9183Block::operator==(const Uic9183Block &other) const
{
    return size() == other.size() && std::memcmp(m_data.constData() + m_offset, other.m_data.constData() + other.m_offset, size()) == 0;
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

bool Uic9183Block::isA(const char recordId[6]) const
{
    return std::strncmp(name(), recordId, 6) == 0;
}

const char* Uic9183Block::content() const
{
    if (isNull()) {
        return nullptr;
    }
    return m_data.constData() + m_offset + BlockHeaderSize;
}

int Uic9183Block::size() const
{
    return Uic9183Utils::readAsciiEncodedNumber(m_data, m_offset + BlockSizeOffset, BlockSizeSize);
}

int Uic9183Block::contentSize() const
{
    return std::max(0, size() - BlockHeaderSize);
}

int Uic9183Block::version() const
{
    return Uic9183Utils::readAsciiEncodedNumber(m_data, m_offset + BlockVersionOffset, BlockVersionSize);
}

bool Uic9183Block::isNull() const
{
    return m_data.isEmpty();
}

Uic9183Block Uic9183Block::nextBlock() const
{
    return Uic9183Block(m_data, m_offset + size());
}

QString Uic9183Block::contentText() const
{
    return Uic9183Utils::readUtf8String(m_data, m_offset + BlockHeaderSize, contentSize());
}

#include "moc_uic9183block.cpp"
