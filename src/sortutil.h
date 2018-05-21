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

#ifndef KITINERARY_SORTUTIL_H
#define KITINERARY_SORTUTIL_H

#include "kitinerary_export.h"

class QDateTime;
class QVariant;

namespace KItinerary {

/** Utility function for sorting reservations/visits/events. */
namespace SortUtil
{
    /** Returns the (start) time associated with the given reservation. */
    KITINERARY_EXPORT QDateTime startDateTime(const QVariant &res);

    /** Returns the (end) time associated with the given reservation. */
    KITINERARY_EXPORT QDateTime endtDateTime(const QVariant &res);

    /** Sorting function for top-level reservation/visit/event elements. */
    KITINERARY_EXPORT bool isBefore(const QVariant &lhs, const QVariant &rhs);
}

}

#endif // KITINERARY_SORTUTIL_H
