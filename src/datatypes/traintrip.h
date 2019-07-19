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
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef KITINERARY_TRAINTRIP_H
#define KITINERARY_TRAINTRIP_H

#include "kitinerary_export.h"
#include "datatypes.h"
#include "organization.h"
#include "place.h"

class QDateTime;

namespace KItinerary {

class TrainTripPrivate;

/** A train trip.
 *  @see https://schema.org/TrainTrip
 */
class KITINERARY_EXPORT TrainTrip
{
    KITINERARY_GADGET(TrainTrip)
    KITINERARY_PROPERTY(QString, arrivalPlatform, setArrivalPlatform)
    KITINERARY_PROPERTY(KItinerary::TrainStation, arrivalStation, setArrivalStation)
    KITINERARY_PROPERTY(QDateTime, arrivalTime, setArrivalTime)
    KITINERARY_PROPERTY(QString, departurePlatform, setDeparturePlatform)
    KITINERARY_PROPERTY(KItinerary::TrainStation, departureStation, setDepartureStation)
    KITINERARY_PROPERTY(QDateTime, departureTime, setDepartureTime)
    KITINERARY_PROPERTY(QString, trainName, setTrainName)
    KITINERARY_PROPERTY(QString, trainNumber, setTrainNumber)
    KITINERARY_PROPERTY(KItinerary::Organization, provider, setProvider)

    // KDE extensions
    /** The scheduled day of departure.
     *  This is needed for unbound train reservations where we don't know the
     *  exact travel details yet.
     *  @see Flight
     */
    KITINERARY_PROPERTY(QDate, departureDay, setDepartureDay)

private:
    QExplicitlySharedDataPointer<TrainTripPrivate> d;
};

}

Q_DECLARE_METATYPE(KItinerary::TrainTrip)

#endif // KITINERARY_TRAINTRIP_H
