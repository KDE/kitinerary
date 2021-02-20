/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "vendor0080block.h"
#include "vendor0080vublockdata.h"
#include "logging.h"

#include <QString>

using namespace KItinerary;

enum {
    SBlockFirstByte = 'S',
    SBlockTypeOffset = 1,
    SBlockLengthOffset = 4,
    SBlockLengthSize = 4,
    SBlockHeaderSize = 8
};

// 0080BL vendor block sub-block ("S block")
// 1x 'S'
// 3x field type
// 4x field value length
// nx field value

Vendor0080BLSubBlock::Vendor0080BLSubBlock() = default;

Vendor0080BLSubBlock::Vendor0080BLSubBlock(const Uic9183Block &block, int offset)
    : m_offset(offset)
{
    if (block.isNull()) {
        return;
    }

    if (block.contentSize() < offset + SBlockHeaderSize) {
        qCWarning(Log) << "0080BL S-block too small";
        return;
    }
    if (*(block.content() + offset) != SBlockFirstByte) {
        qCWarning(Log) << "0080BL invalid S-block header";
        return;
    }

    m_block = block;
    if (block.contentSize() < offset + size()) {
        qCWarning(Log) << "0080BL S-block size exceeds 0080BL block size";
        m_block = {};
    }
}

bool Vendor0080BLSubBlock::isNull() const
{
    return m_block.isNull();
}

int Vendor0080BLSubBlock::size() const
{
    return contentSize() + SBlockHeaderSize;
}

Vendor0080BLSubBlock Vendor0080BLSubBlock::nextBlock() const
{
    if (m_offset + size() >= m_block.contentSize()) { // we are the last block
        return {};
    }
    return Vendor0080BLSubBlock(m_block, m_offset + size());
}

int Vendor0080BLSubBlock::contentSize() const
{
    if (isNull()) {
        return 0;
    }
    return QByteArray(m_block.content() + m_offset + SBlockLengthOffset, SBlockLengthSize).toInt();
}

const char* Vendor0080BLSubBlock::id() const
{
    if (isNull()) {
        return nullptr;
    }
    return m_block.content() + m_offset + SBlockTypeOffset;
}

const char* Vendor0080BLSubBlock::content() const
{
    if (isNull()) {
        return nullptr;
    }
    return m_block.content() + m_offset + SBlockHeaderSize;
}

QString Vendor0080BLSubBlock::toString() const
{
    if (isNull()) {
        return {};
    }
    return QString::fromUtf8(content(), contentSize());
}


// 0080BL vendor block (DB) (version 2/3, dynamic size)
// 2x stuff
// 1x number of certificate blocks
// 22+8+8+8x (v2) or 8+8+10x (v3) certificate block
// 2x number of sub blocks

Vendor0080BLBlock::Vendor0080BLBlock(const Uic9183Block &block)
{
    if (block.isNull()) {
        return;
    }
    if (block.version() != 2 && block.version() != 3) {
        qCWarning(Log) << "Unsupported version of 0080BL vendor block." << block.version();
        return;
    }
    if (block.isNull() || block.contentSize() < 3 || subblockOffset(block) > block.size()) {
        return;
    }
    m_block = block;
}

bool Vendor0080BLBlock::isValid() const
{
    return !m_block.isNull();
}

Vendor0080BLSubBlock Vendor0080BLBlock::firstBlock() const
{
    return Vendor0080BLSubBlock(m_block, subblockOffset(m_block));
}

Vendor0080BLSubBlock Vendor0080BLBlock::findSubBlock(const char id[3]) const
{
    auto sblock = firstBlock();
    while (!sblock.isNull()) {
        if (strncmp(sblock.id(), id, 3) == 0) {
            return sblock;
        }
        sblock = sblock.nextBlock();
    }
    return {};
}

int Vendor0080BLBlock::subblockOffset(const Uic9183Block& block)
{
    const auto certCount = *(block.content() + 2) - '0';
    const auto certSize = block.version() == 2 ? 46 : 26;
    return 3 + certSize * certCount + 2;
}


Vendor0080VUBlock::Vendor0080VUBlock(const Uic9183Block &block)
{
    if (block.isNull() || block.contentSize() < (int)sizeof(Vendor0080VUCommonData)) {
        return;
    }

    m_block = block;
}

bool Vendor0080VUBlock::isValid() const
{
    return !m_block.isNull();
}

const Vendor0080VUCommonData* Vendor0080VUBlock::commonData() const
{
    return m_block.isNull() ? nullptr : reinterpret_cast<const Vendor0080VUCommonData*>(m_block.content());
}

const Vendor0080VUTicketData* Vendor0080VUBlock::ticketData(int index) const
{
    auto offset = sizeof(Vendor0080VUCommonData);
    auto ticket = reinterpret_cast<const Vendor0080VUTicketData*>(m_block.content() + offset);
    while (index-- > 0) {
        offset += sizeof(Vendor0080VUTicketData) + ticket->validityAreaDataSize - sizeof(VdvTicketValidityAreaData);
        ticket = reinterpret_cast<const Vendor0080VUTicketData*>(m_block.content() + offset);
    }
    return ticket;
}
