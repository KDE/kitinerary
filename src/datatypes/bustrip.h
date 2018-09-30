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

#ifndef KITINERARY_BUSTRIP_H
#define KITINERARY_BUSTRIP_H

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

    [[deprecated]] inline KItinerary::BusStation arrivalStation() const { return arrivalBusStop(); }
    [[deprecated]] inline KItinerary::BusStation departureStation() const { return departureBusStop(); }
    Q_PROPERTY(KItinerary::BusStation arrivalStation READ arrivalStation STORED false)
    Q_PROPERTY(KItinerary::BusStation departureStation READ departureStation STORED false)
private:
    QExplicitlySharedDataPointer<BusTripPrivate> d;
};

}

Q_DECLARE_METATYPE(KItinerary::BusTrip)

#endif // KITINERARY_BUSTRIP_H
