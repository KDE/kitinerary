/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "vendor1154block.h"
#include "uic9183utils.h"
#include "logging.h"

using namespace KItinerary;

// 1154UT vendor block sub-block format
// 2x field type
// 3x field size as ASCII number
// nx field content
enum {
    SubBlockTypeOffset = 0,
    SubBlockTypeSize = 2,
    SubBlockLengthOffset = SubBlockTypeOffset + SubBlockTypeSize,
    SubBlockLengthSize = 3,
    SubBlockHeaderSize = SubBlockTypeSize + SubBlockLengthSize,
    SubBlockContentOffset = SubBlockLengthOffset + SubBlockLengthSize,
};

// known field types:
// KJ: passenger name
// OD: begin of validity
// DO: end of validity
// EM: ?
// KD: type of card (0 == none?)
// KC: card number
// KK: transaction code
// KS: routing, as a | separated list of UIC station codes?
// KM: distance in kilometers

Vendor1154UTSubBlock::Vendor1154UTSubBlock() = default;

Vendor1154UTSubBlock::Vendor1154UTSubBlock(const Uic9183Block &block, int offset)
    : m_offset(offset)
{
    if (block.isNull()) {
        return;
    }

    if (block.contentSize() < offset + SubBlockHeaderSize) {
        qCWarning(Log) << "1154UT sub-block too small";
        return;
    }

    m_block = block;
    if (block.contentSize() < offset + size()) {
        qCWarning(Log) << "1154UT sub-block size exceeds 1154UT block size";
        m_block = {};
    }
}

bool Vendor1154UTSubBlock::isNull() const
{
    return m_block.isNull();
}

int Vendor1154UTSubBlock::size() const
{
    return contentSize() + SubBlockHeaderSize;
}

Vendor1154UTSubBlock Vendor1154UTSubBlock::nextBlock() const
{
    if (m_offset + size() >= m_block.contentSize()) { // we are the last block
        return {};
    }
    return Vendor1154UTSubBlock(m_block, m_offset + size());
}

int Vendor1154UTSubBlock::contentSize() const
{
    if (isNull()) {
        return 0;
    }
    return Uic9183Utils::readAsciiEncodedNumber(m_block.content(), m_block.size(), m_offset + SubBlockLengthOffset, SubBlockLengthSize);
}

const char* Vendor1154UTSubBlock::id() const
{
    if (isNull()) {
        return nullptr;
    }
    return m_block.content() + m_offset + SubBlockTypeOffset;
}

const char* Vendor1154UTSubBlock::content() const
{
    if (isNull()) {
        return nullptr;
    }
    return m_block.content() + m_offset + SubBlockHeaderSize;
}

QString Vendor1154UTSubBlock::toString() const
{
    if (isNull()) {
        return {};
    }
    return QString::fromUtf8(content(), contentSize());
}

Vendor1154UTBlock::Vendor1154UTBlock(const Uic9183Block &block)
    : m_block(block)
{
}

bool Vendor1154UTBlock::isValid() const
{
    return !m_block.isNull();
}

Vendor1154UTSubBlock Vendor1154UTBlock::firstBlock() const
{
    return Vendor1154UTSubBlock(m_block, 0);
}

Vendor1154UTSubBlock Vendor1154UTBlock::findSubBlock(const char id[SubBlockTypeSize]) const
{
    auto sblock = firstBlock();
    while (!sblock.isNull()) {
        if (strncmp(sblock.id(), id, SubBlockTypeSize) == 0) {
            return sblock;
        }
        sblock = sblock.nextBlock();
    }
    return {};
}

QVariant Vendor1154UTBlock::findSubBlock(const QString &str) const
{
    if (str.size() != 3 || !isValid()) {
        return {};
    }
    const auto b = findSubBlock(str.toUtf8().constData());
    return b.isNull() ? QVariant() : QVariant::fromValue(b);
}
