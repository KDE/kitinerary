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

class FlightPrivate;

/** A flight.
 *  @see https://schema.org/Flight
 *  @see https://developers.google.com/gmail/markup/reference/flight-reservation
 */
class KITINERARY_EXPORT Flight
{
    KITINERARY_GADGET(Flight)
    KITINERARY_PROPERTY(QString, flightNumber, setFlightNumber)
    KITINERARY_PROPERTY(KItinerary::Airline, airline, setAirline)
    KITINERARY_PROPERTY(KItinerary::Airport, departureAirport, setDepartureAirport)
    KITINERARY_PROPERTY(QString, departureGate, setDepartureGate)
    KITINERARY_PROPERTY(QString, departureTerminal, setDepartureTerminal)
    KITINERARY_PROPERTY(QDateTime, departureTime, setDepartureTime)
    KITINERARY_PROPERTY(KItinerary::Airport, arrivalAirport, setArrivalAirport)
    KITINERARY_PROPERTY(QString, arrivalTerminal, setArrivalTerminal)
    KITINERARY_PROPERTY(QDateTime, arrivalTime, setArrivalTime)
    KITINERARY_PROPERTY(KItinerary::Organization, provider, setProvider)

    // Google extension for boarding pass data
    KITINERARY_PROPERTY(QDateTime, boardingTime, setBoardingTime)

    // KDE extensions
    /** The scheduled day of departure.
     *  This is part of the unique identification of a flight and part of the IATA BCBP data.
     *  This might be different from departureTime, which reflects the actual time of departure
     *  and thus can in case of delays even move to a following day.
     */
    KITINERARY_PROPERTY(QDate, departureDay, setDepartureDay)

private:
    QExplicitlySharedDataPointer<FlightPrivate> d;
};

}

Q_DECLARE_METATYPE(KItinerary::Flight)

