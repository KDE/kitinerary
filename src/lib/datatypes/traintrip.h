/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

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

