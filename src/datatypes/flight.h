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

class Airport;

class AirlinePrivate;

/** An airline.
 *  @see https://schema.org/Airline
 */
class KITINERARY_EXPORT Airline
{
    KITINERARY_GADGET(Airline)
    KITINERARY_PROPERTY(QString, name, setName)
    KITINERARY_PROPERTY(QString, iataCode, setIataCode)
private:
    detail::shared_data_ptr<AirlinePrivate> d;
};

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
    KITINERARY_PROPERTY(QDateTime, departureTime, setDepartureTime)
    KITINERARY_PROPERTY(KItinerary::Airport, arrivalAirport, setArrivalAirport)
    KITINERARY_PROPERTY(QDateTime, arrivalTime, setArrivalTime)

    // Google extension for boarding pass data
    KITINERARY_PROPERTY(QDateTime, boardingTime, setBoardingTime)
    KITINERARY_PROPERTY(QString, departureGate, setDepartureGate)

    Q_PROPERTY(QString departureTimeLocalized READ departureTimeLocalized STORED false CONSTANT)
    Q_PROPERTY(QString arrivalTimeLocalized READ arrivalTimeLocalized STORED false CONSTANT)
    Q_PROPERTY(QString boardingTimeLocalized READ boardingTimeLocalized STORED false CONSTANT)
private:
    QString departureTimeLocalized() const;
    QString arrivalTimeLocalized() const;
    QString boardingTimeLocalized() const;

    detail::shared_data_ptr<FlightPrivate> d;
};

}

Q_DECLARE_METATYPE(KItinerary::Airline)
Q_DECLARE_METATYPE(KItinerary::Flight)

#endif // KITINERARY_FLIGHT_H
