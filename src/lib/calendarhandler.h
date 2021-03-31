/*
   SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kitinerary_export.h"

#include <QSharedPointer>
template <typename T> class QVector;

class QVariant;

namespace KCalendarCore {
class Calendar;
class Event;
}

namespace KItinerary {

/** Methods for converting between ical events and JSON-LD booking data. */
namespace CalendarHandler
{
    /** Attempts to find calendar events in @p calendar for @p reservation.
     *  For a complete reservation this should not return more than one element,
     *  for a minimal cancellation element however this can return multiple events
     *  (e.g. all trip segments covered by the same reservation number).
     *  @since 20.08
     */
    KITINERARY_EXPORT QVector<QSharedPointer<KCalendarCore::Event>> findEvents(const QSharedPointer<KCalendarCore::Calendar> &calendar, const QVariant &reservation);

    /** Attempts to find an event in @p calendar for @p reservation.
     *  @deprecated since 20.08 use findEvents instead.
     */
    KITINERARY_DEPRECATED_EXPORT QSharedPointer<KCalendarCore::Event> findEvent(const QSharedPointer<KCalendarCore::Calendar> &calendar, const QVariant &reservation);

    /** Returns the reservations for this event.
     *  In case of a mult-travler trip, the result contains more than one reservation.
     */
    KITINERARY_EXPORT QVector<QVariant> reservationsForEvent(const QSharedPointer<KCalendarCore::Event> &event);

    /** Checks if the given @p reservation contains enough information to create an iCal event from it. */
    KITINERARY_EXPORT bool canCreateEvent(const QVariant &reservation);

    /** Fills @p event with details of @p reservations.
     *  Can be used on new events or to update existing ones.
     *  @param reservations When passing more than one reservation here, those must be for the same multi-traveler trip.
     *  That is, MergeUtil::isSame() returns true for Reservation::reservationFor for each pair of values.
     *  @param event The event.
     */
    KITINERARY_EXPORT void fillEvent(const QVector<QVariant> &reservations, const QSharedPointer<KCalendarCore::Event> &event);
}

}

