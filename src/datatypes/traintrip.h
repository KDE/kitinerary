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

#ifndef KITINERARY_TRAINTRIP_H
#define KITINERARY_TRAINTRIP_H

#include "kitinerary_export.h"
#include "datatypes.h"

class QDateTime;

namespace KItinerary {

class TrainStation;

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
    KITINERARY_PROPERTY(KItinerary::TrainStation, departureStation, setDeparatureStation)
    KITINERARY_PROPERTY(QDateTime, departureTime, setDepartureTime)
    KITINERARY_PROPERTY(QString, trainName, setTrainName)
    KITINERARY_PROPERTY(QString, trainNumber, setTrainNumber)

    Q_PROPERTY(QString departureTimeLocalized READ departureTimeLocalized STORED false CONSTANT)
    Q_PROPERTY(QString arrivalTimeLocalized READ arrivalTimeLocalized STORED false CONSTANT)

private:
    QString departureTimeLocalized() const;
    QString arrivalTimeLocalized() const;

    QExplicitlySharedDataPointer<TrainTripPrivate> d;
};

}

Q_DECLARE_METATYPE(KItinerary::TrainTrip)

#endif // KITINERARY_TRAINTRIP_H
