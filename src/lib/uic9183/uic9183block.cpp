/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "uic9183block.h"
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

QString Uic9183Block::contentText() const
{
    return QString::fromUtf8(content(), contentSize());
}

int Uic9183Block::readAsciiEncodedNumber(int offset, int length) const
{
    if (isNull() || offset < 0 || length < 1 || m_offset + BlockHeaderSize + offset + length > m_data.size()) {
        qCWarning(Log) << "invalid UIC 918.3 block content read:" << offset << length << m_offset << m_data.size();
        return {};
    }

    int v = 0;
    for (int i = 0; i < length; ++i) {
        v *= 10;
        v += (*(content() + offset + i)) - '0';
    }
    return v;
}

QString Uic9183Block::readUtf8String(int offset, int length) const
{
    if (isNull() || offset < 0 || length < 1 || m_offset + BlockHeaderSize + offset + length > m_data.size()) {
        qCWarning(Log) << "invalid UIC 918.3 block content read:" << offset << length << m_offset << m_data.size();
        return {};
    }
    return QString::fromUtf8(content() + offset, length);
}