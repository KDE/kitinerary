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

#include "vendor0080block.h"
#include "logging.h"

#include <QString>

using namespace KItinerary;

// 0080BL vendor block sub-block ("S block")
// 1x 'S'
// 3x field type
// 4x field value length
// nx field value

Vendor0080BLSubBlock::Vendor0080BLSubBlock() = default;

Vendor0080BLSubBlock::Vendor0080BLSubBlock(const char *data, int size)
    : m_data(data)
    , m_size(size)
{
}

bool Vendor0080BLSubBlock::isNull() const
{
    return m_size <= 0 || !m_data;
}

int Vendor0080BLSubBlock::size() const
{
    return m_size;
}

const char* Vendor0080BLSubBlock::id() const
{
    return m_data + 1;
}

const char* Vendor0080BLSubBlock::data() const
{
    return m_data + 8;
}

QString Vendor0080BLSubBlock::toString() const
{
    return QString::fromUtf8(data(), size());
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

Vendor0080BLSubBlock Vendor0080BLBlock::findSubBlock(const char id[3]) const
{
    for (int i = subblockOffset(m_block); i < m_block.contentSize();) {
        if (*(m_block.content() + i) != 'S') {
            qCWarning(Log) << "0080BL invalid S-block format.";
            return {};
        }
        const int subblockSize = QByteArray(m_block.content() + i + 4, 4).toInt();
        if (subblockSize + i > m_block.size()) {
            qCWarning(Log) << "0080BL S-block size exceeds block size.";
            return {};
        }
        Vendor0080BLSubBlock sb(m_block.content() + i, subblockSize);
        if (!sb.isNull() && strncmp(sb.id(), id, 3) == 0) {
            return sb;
        }
        i += subblockSize + 8;
    }
    return {};
}

int Vendor0080BLBlock::subblockOffset(const Uic9183Block& block)
{
    const auto certCount = *(block.content() + 2) - '0';
    const auto certSize = block.version() == 2 ? 46 : 26;
    return 3 + certSize * certCount + 2;
}
