/*
    SPDX-FileCopyrightText: 2017-2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "flightutil_p.h"

#include <QDebug>

using namespace KItinerary;

enum {
    AirplaneSpeedLowerBound = 250, // km/h, turboprop aircraft, and a bit lower than average cruise speed to account for takeoff/landing
    AirplaneSpeedUpperBound = 2140, // km/h, Concorde, so a bit excessive
};

bool FlightUtil::isPlausibleDistanceForDuration(int distance, int duration)
{
    int lowerBoundDistance = AirplaneSpeedLowerBound * (duration / 3.6);
    if (duration < 3600) { // for ultra short flights the takeoff/landing overhead is so big that our expected speed range doesn't work reliable anymore
        lowerBoundDistance /= 2;
    }
    const int upperBoundDistance = AirplaneSpeedUpperBound * (duration / 3.6);

    return distance < upperBoundDistance && distance > lowerBoundDistance;
}
