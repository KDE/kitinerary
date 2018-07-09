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

#include "flight.h"
#include "place.h"
#include "organization.h"
#include "datatypes_p.h"

#include <QDateTime>
#include <QVariant>

using namespace KItinerary;

namespace KItinerary {

class FlightPrivate : public QSharedData
{
public:
    QString flightNumber;
    Airline airline;
    Airport departureAirport;
    QString departureGate;
    QString departureTerminal;
    QDateTime departureTime;
    Airport arrivalAirport;
    QDateTime arrivalTime;
    QDateTime boardingTime;
    QDate departureDay;
    Organization provider;
};

KITINERARY_MAKE_SIMPLE_CLASS(Flight)
KITINERARY_MAKE_PROPERTY(Flight, QString, flightNumber, setFlightNumber)
KITINERARY_MAKE_PROPERTY(Flight, Airline, airline, setAirline)
KITINERARY_MAKE_PROPERTY(Flight, Airport, departureAirport, setDepartureAirport)
KITINERARY_MAKE_PROPERTY(Flight, QString, departureGate, setDepartureGate)
KITINERARY_MAKE_PROPERTY(Flight, QString, departureTerminal, setDepartureTerminal)
KITINERARY_MAKE_PROPERTY(Flight, QDateTime, departureTime, setDepartureTime)
KITINERARY_MAKE_PROPERTY(Flight, Airport, arrivalAirport, setArrivalAirport)
KITINERARY_MAKE_PROPERTY(Flight, QDateTime, arrivalTime, setArrivalTime)
KITINERARY_MAKE_PROPERTY(Flight, QDateTime, boardingTime, setBoardingTime)
KITINERARY_MAKE_PROPERTY(Flight, Organization, provider, setProvider)

QDate Flight::departureDay() const
{
    if (d->departureDay.isValid()) {
        return d->departureDay;
    }
    // pre-1970 dates are used as transient state when we only know the time
    if (d->departureTime.isValid() && d->departureTime.date().year() > 1970) {
        return d->departureTime.date();
    }
    if (d->boardingTime.isValid() && d->boardingTime.date().year() > 1970) {
        return d->boardingTime.date();
    }
    return {};
}

void Flight::setDepartureDay(const QDate &value)
{
    d.detach();
    d->departureDay = value;
}

}

#include "moc_flight.cpp"
