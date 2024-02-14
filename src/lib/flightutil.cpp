/*
    SPDX-FileCopyrightText: 2017-2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "flightutil_p.h"

#include <QDebug>

using namespace KItinerary;

constexpr inline const auto AirplaneSpeedLowerBound = 250; // km/h, turboprop aircraft, and a bit lower than average cruise speed to account for takeoff/landing
constexpr inline const auto AirplaneSpeedUpperBound = 2140; // km/h, Concorde, so a bit excessive

bool FlightUtil::isPlausibleDistanceForDuration(int distance, std::chrono::seconds duration)
{
    auto lowerBoundDistance = AirplaneSpeedLowerBound * ((double)duration.count() / 3.6);
    if (duration < std::chrono::hours(1)) { // for ultra short flights the takeoff/landing overhead is so big that our expected speed range doesn't work reliable anymore
        lowerBoundDistance /= 2;
    }
    const auto upperBoundDistance = AirplaneSpeedUpperBound * ((double)duration.count() / 3.6);

    return distance < upperBoundDistance && distance > lowerBoundDistance;
}
