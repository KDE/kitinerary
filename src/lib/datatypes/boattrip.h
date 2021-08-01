/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kitinerary_export.h"
#include "datatypes.h"
#include "place.h"

class QDateTime;

namespace KItinerary {

class BoatTripPrivate;

/** A boat or ferry trip.
 *  @see https://schema.org/BoatTrip
 */
class KITINERARY_EXPORT BoatTrip
{
    KITINERARY_GADGET(BoatTrip)
    KITINERARY_PROPERTY(QString, name, setName)
    KITINERARY_PROPERTY(KItinerary::BoatTerminal, arrivalBoatTerminal, setArrivalBoatTerminal)
    KITINERARY_PROPERTY(QDateTime, arrivalTime, setArrivalTime)
    KITINERARY_PROPERTY(KItinerary::BoatTerminal, departureBoatTerminal, setDepartureBoatTerminal)
    KITINERARY_PROPERTY(QDateTime, departureTime, setDepartureTime)

private:
    QExplicitlySharedDataPointer<BoatTripPrivate> d;
};

}

Q_DECLARE_METATYPE(KItinerary::BoatTrip)

