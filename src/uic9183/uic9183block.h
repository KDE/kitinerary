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

#ifndef KITINERARY_UIC9183BLOCK_H
#define KITINERARY_UIC9183BLOCK_H

#include "kitinerary_export.h"

#include <QByteArray>

namespace KItinerary {

/** A data block from a UIC 918.3 ticket. */
class KITINERARY_EXPORT Uic9183Block
{
public:
    Uic9183Block();
    explicit Uic9183Block(const QByteArray &data, int offset);
    Uic9183Block(const Uic9183Block&);
    Uic9183Block(Uic9183Block&&);
    Uic9183Block& operator=(const Uic9183Block&);
    Uic9183Block& operator=(Uic9183Block&&);

    /** Returns the block name (6 characters).
     *  The name is either "U_" + 4 letter standard type or a 4 digit vendor id + 2 char vendor type
     */
    const char *name() const;
    /** Returns the payload data (not including the block header). */
    const char *content() const;
    /** Returns the raw payload data. */
    KITINERARY_DEPRECATED const char *data() const;
    /** Returns the size of the entire block data. */
    int size() const;
    /** Returns the size of the content data. */
    int contentSize() const;
    /** Returns the version number of this block. */
    int version() const;

    /** Checks if the block is valid or empty/default constructed. */
    bool isNull() const;

    /** Returns the next block in the ticket.
     *  If there is no more block, a null block is returned.
     */
    Uic9183Block nextBlock() const;

private:
    QByteArray m_data;
    int m_offset = 0;
};

}

#endif // KITINERARY_UIC9183BLOCK_H
