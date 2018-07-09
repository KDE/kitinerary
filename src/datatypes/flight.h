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

#ifndef KITINERARY_FLIGHT_H
#define KITINERARY_FLIGHT_H

#include "kitinerary_export.h"
#include "datatypes.h"

class QDateTime;

namespace KItinerary {

class Airline;
class Airport;
class Organization;

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

#endif // KITINERARY_FLIGHT_H
