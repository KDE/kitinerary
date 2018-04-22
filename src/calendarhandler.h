/*
   Copyright (c) 2017 Volker Krause <vkrause@kde.org>

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   This library is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
   License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.
*/

#ifndef CALENDARHANDLER_H
#define CALENDARHANDLER_H

#include "kitinerary_export.h"

#include <KCalCore/Calendar>
#include <KCalCore/Event>

class QVariant;

namespace KItinerary {

/** Methods for converting between ical events and JSON-LD booking data. */
namespace CalendarHandler
{
    /** Returns the start time associated with the given reservation. */
    KITINERARY_EXPORT QDateTime startDateTime(const QVariant &reservation);

    /** Attempts to find an event in @p calendar for @p reservation. */
    KITINERARY_EXPORT KCalCore::Event::Ptr findEvent(const KCalCore::Calendar::Ptr &calendar, const QVariant &reservation);

    /** Returns the reservation object for this event. */
    KITINERARY_EXPORT QVariant reservationForEvent(const KCalCore::Event::Ptr &event);

    /** Fills @p event with details of @p reservation.
     *  Can be used on new events or to update existing ones.
     */
    KITINERARY_EXPORT void fillEvent(const QVariant &reservation, const KCalCore::Event::Ptr &event);
}

}

#endif // CALENDARHANDLER_H
