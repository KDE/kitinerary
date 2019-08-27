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

#ifndef KITINERARY_VENDOR0080BLOCK_H
#define KITINERARY_VENDOR0080BLOCK_H

#include "uic9183block.h"

class QString;

namespace KItinerary {

/** UIC 918.3 0080BL vendor data block sub-block. */
class Vendor0080BLSubBlock
{
public:
    Vendor0080BLSubBlock();
    Vendor0080BLSubBlock(const char *data, int size);

    bool isNull() const;
    int size() const;
    const char *id() const;
    const char *data() const;

    QString toString() const;

private:
    const char *m_data = nullptr;
    int m_size = 0;
};


/** UIC 918.3 0080BL vendor data block. */
class Vendor0080BLBlock
{
public:
    Vendor0080BLBlock(const Uic9183Block &block);

    bool isValid() const;
    Vendor0080BLSubBlock findSubBlock(const char id[3]) const;

private:
    static int subblockOffset(const Uic9183Block &block);

    Uic9183Block m_block;
};

}

#endif // KITINERARY_VENDOR0080BLOCK_H
