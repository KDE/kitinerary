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

#include <QSharedPointer>
template <typename T> class QVector;

class QDateTime;
class QVariant;

namespace KCalendarCore {
class Calendar;
class Event;
}

namespace KItinerary {

/** Methods for converting between ical events and JSON-LD booking data. */
namespace CalendarHandler
{
    /** Attempts to find an event in @p calendar for @p reservation. */
    KITINERARY_EXPORT QSharedPointer<KCalendarCore::Event> findEvent(const QSharedPointer<KCalendarCore::Calendar> &calendar, const QVariant &reservation);

    /** Returns the reservations for this event.
     *  In case of a mult-travler trip, the result contains more than one reservation.
     */
    KITINERARY_EXPORT QVector<QVariant> reservationsForEvent(const QSharedPointer<KCalendarCore::Event> &event);

    /** Fills @p event with details of @p reservations.
     *  Can be used on new events or to update existing ones.
     *  @param reservations When passing more than one reservation here, those most be for the same multi-traveler trip.
     *  @param event The event.
     *  That is, MergeUtil::isSame() returns true for Reservation::reservationFor for each pair of values.
     */
    KITINERARY_EXPORT void fillEvent(const QVector<QVariant> &reservations, const QSharedPointer<KCalendarCore::Event> &event);
}

}

#endif // CALENDARHANDLER_H
