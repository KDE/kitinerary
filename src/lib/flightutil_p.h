/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_FLIGHTUTIL_H
#define KITINERARY_FLIGHTUTIL_H

namespace KItinerary {

/** Utility functions for dealing with flights. */
namespace FlightUtil
{
/** Check whether the given distance (in meters) can be covered by a flight
 *  in the given time (in seconds).
 */
bool isPlausibleDistanceForDuration(int distance, int duration);

}

}

#endif // KITINERARY_FLIGHTUTIL_H
