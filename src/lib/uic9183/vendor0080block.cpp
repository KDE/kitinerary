/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "vendor0080block.h"
#include "vendor0080vublockdata.h"
#include "uic9183utils.h"
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

// S001: price model (textual)
// S002: product class (numeric: 0 = C, 1 = B, 2 = A)
// S003: product class outbound (A, B, or C)
// S004: produce class return
// S009: [num adults]-[num adults with Bahncard]-[Bahncard type] (49 = BC25, 19,78 = BC50)
// S012: number of children
// S014: S[class of travel]
// S015: departure station name (outbound)
// S016: arrival station name (outbound)
// S017: departure station name (return)
// S018: arrival station name (return)
// S019, S020: Rail&Fly specific
// S021: via
// S023: traveler name
// S026: price type (numeric) (12 = Normalpreis, 13 = Sparpreis, 3 = Rail&Fly)
// S027: id number (no longer used)
// S028: [given name]#[family name]
// S031: first day of validity, dd.MM.yyyy
// S032: last day of validity, dd.MM.yyyy
// S035: departure station IBNR without country prefix (outbound)
// S036: arrival station IBNR without country prefix (outbound)
// S040: number of persons?
// S041: number of tickets?

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
    return Uic9183Utils::readAsciiEncodedNumber(m_block.content(), m_block.size(), m_offset + SBlockLengthOffset, SBlockLengthSize);
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


Vendor0080BLOrderBlock::Vendor0080BLOrderBlock() = default;
Vendor0080BLOrderBlock::Vendor0080BLOrderBlock(const Uic9183Block &block, int offset)
    : m_block(block)
    , m_offset(offset)
{
}

bool Vendor0080BLOrderBlock::isNull() const
{
    return m_block.isNull();
}

QDate Vendor0080BLOrderBlock::validFrom() const
{
    switch (m_block.version()) {
        case 2:
            return QDate::fromString(Uic9183Utils::readUtf8String(m_block, m_offset + 22, 8), QStringLiteral("ddMMyyyy"));
        case 3:
            return QDate::fromString(Uic9183Utils::readUtf8String(m_block, m_offset, 8), QStringLiteral("ddMMyyyy"));
    }
    return {};
}

QDate Vendor0080BLOrderBlock::validTo() const
{
    switch (m_block.version()) {
        case 2:
            return QDate::fromString(Uic9183Utils::readUtf8String(m_block, m_offset + 22 + 8, 8), QStringLiteral("ddMMyyyy"));
        case 3:
            return QDate::fromString(Uic9183Utils::readUtf8String(m_block, m_offset + 8, 8), QStringLiteral("ddMMyyyy"));
    }
    return {};
}

QString Vendor0080BLOrderBlock::serialNumber() const
{
    switch (m_block.version()) {
        case 2:
            return Uic9183Utils::readUtf8String(m_block, m_offset + 22 + 8 + 8, 8);
        case 3:
            return Uic9183Utils::readUtf8String(m_block, m_offset + 8 + 8, 10);
    }
    return {};
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

int Vendor0080BLBlock::orderBlockCount() const
{
    return Uic9183Utils::readAsciiEncodedNumber(m_block, 2, 1);
}

Vendor0080BLOrderBlock Vendor0080BLBlock::orderBlock(int i) const
{
    if (i >= 0 && i < orderBlockCount()) {
        switch (m_block.version()) {
            case 2: return Vendor0080BLOrderBlock(m_block, 3 + i * (22 + 8 + 8 + 8));
            case 3: return Vendor0080BLOrderBlock(m_block, 3 + i * (8 + 8 + 10));
        }
    }
    return {};
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

QVariant Vendor0080BLBlock::findSubBlock(const QString &str) const
{
    if (str.size() != 3 || !isValid()) {
        return {};
    }
    const auto b = findSubBlock(str.toUtf8().constData());
    return b.isNull() ? QVariant() : QVariant::fromValue(b);
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

#include "moc_vendor0080block.cpp"
