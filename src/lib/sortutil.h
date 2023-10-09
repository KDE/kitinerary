/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kitinerary_export.h"

class QDateTime;
class QVariant;

namespace KItinerary {

/** Utility function for sorting reservations/visits/events. */
namespace SortUtil
{
    /** Returns the (start) time associated with the given element.
     *  This can be either a Reservation object, or a reservable object
     *  if that has a time associated (such as an Event).
     */
    KITINERARY_EXPORT QDateTime startDateTime(const QVariant &elem);

    /** Returns the (end) time associated with the given element.
     *  This can be either a Reservation object, or a reservable object
     *  if that has a time associated (such as an Event).
     */
    KITINERARY_EXPORT QDateTime endDateTime(const QVariant &res);

    /** Sorting function for top-level reservation/visit/event elements. */
    KITINERARY_EXPORT bool isBefore(const QVariant &lhs, const QVariant &rhs);

    /** Returns whether the given element has a start time.
     *  This can be @c false even is SortUtil::startDateTime returns a valid
     *  result, if there is only a start date available.
     */
    KITINERARY_EXPORT bool hasStartTime(const QVariant &elem);
    /** Returns whether the given element has an end time.
     *  @see hasStartTime
     */
    KITINERARY_EXPORT bool hasEndTime(const QVariant &elem);
}

}

