/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kitinerary_export.h"
#include "datatypes.h"
#include "organization.h"
#include "place.h"

namespace KItinerary {

class BusTripPrivate;

/** A bus trip.
 *  @see https://schema.org/BusTrip
 */
class KITINERARY_EXPORT BusTrip
{
    KITINERARY_GADGET(BusTrip)
    KITINERARY_PROPERTY(QString, arrivalPlatform, setArrivalPlatform) // ### is this used? it's not in the schema
    KITINERARY_PROPERTY(KItinerary::BusStation, arrivalBusStop, setArrivalBusStop)
    KITINERARY_PROPERTY(QDateTime, arrivalTime, setArrivalTime)
    KITINERARY_PROPERTY(QString, departurePlatform, setDeparturePlatform) // ### not in the schema
    KITINERARY_PROPERTY(KItinerary::BusStation, departureBusStop, setDepartureBusStop)
    KITINERARY_PROPERTY(QDateTime, departureTime, setDepartureTime)
    KITINERARY_PROPERTY(QString, busName, setBusName)
    KITINERARY_PROPERTY(QString, busNumber, setBusNumber)
    KITINERARY_PROPERTY(KItinerary::Organization, provider, setProvider)

private:
    QExplicitlySharedDataPointer<BusTripPrivate> d;
};

}

Q_DECLARE_METATYPE(KItinerary::BusTrip)

