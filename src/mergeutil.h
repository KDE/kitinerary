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

#ifndef KITINERARY_MERGEUTIL_H
#define KITINERARY_MERGEUTIL_H

#include "kitinerary_export.h"

class QVariant;

namespace KItinerary {
class Flight;
class Person;
class TrainTrip;

/** Utilities for merging reservations or elements of them. */
namespace MergeUtil
{
/**
 *  Checks if two reservations refer to the same booking element.
 *
 *  This depends on the reservation type:
 *  - Flights: booking reference, flight number and departure day match
 *  - Train trip: booking reference, train number and departure day match
 *  - But trip: booking ref and/or number and departure time match
 *  - Hotel booking: hotel name, booking reference and checkin day match
 *
 *  For all reservation types, the Reservation::underName property is
 *  checked and either needs to be equal or absent in one of the values.
 */
KITINERARY_EXPORT bool isSameReservation(const QVariant &lhs, const QVariant &rhs);

/**
 * Checks if two Person objects refer to the same person.
 *
 * Essentially a case-insensitive comparisson of the name components.
 */
KITINERARY_EXPORT bool isSamePerson(const Person &lhs, const Person &rhs);
}

}

#endif // KITINERARY_MERGEUTIL_H
