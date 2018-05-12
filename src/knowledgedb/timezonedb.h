/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KITINERARY_TIMEZONEDB_H
#define KITINERARY_TIMEZONEDB_H

#include <cstdint>

class QTimeZone;

namespace KItinerary {
namespace KnowledgeDb {

    /** Timezone type as used in the database.
     *  @note Do not use in API/ABI.
     */
    struct Timezone {
        /** Returns the corresponding QTimeZone. */
        QTimeZone toQTimeZone() const;

        // offset into timezone name string table
        uint16_t offset;
    };
}
}

#endif
