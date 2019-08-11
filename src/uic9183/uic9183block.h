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

namespace KItinerary {

/** A data block from a UIC 918.3 ticket. */
class KITINERARY_EXPORT Uic9183Block
{
public:
    Uic9183Block();
    Uic9183Block(const char *data, int size);
    Uic9183Block(const Uic9183Block&);
    Uic9183Block(Uic9183Block&&);
    Uic9183Block& operator=(const Uic9183Block&);
    Uic9183Block& operator=(Uic9183Block&&);

    /** Returns the raw payload data. */
    const char *data() const;
    /** Returns the size of the raw payload data. */
    int size() const;
    /** Returns the version number of this block. */
    int version() const;

    /** Checks if the block is valid or empty/default constructed. */
    bool isNull() const;

private:
    const char *m_data = nullptr;
    int m_size = 0;
};

}

#endif // KITINERARY_UIC9183BLOCK_H
