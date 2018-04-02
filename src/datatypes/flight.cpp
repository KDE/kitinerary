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
#include "datatypes_p.h"

#include <QVariant>

using namespace KItinerary;

namespace KItinerary {

class AirlinePrivate : public QSharedData
{
public:
    QString name;
    QString iataCode;
};

KITINERARY_MAKE_SIMPLE_CLASS(Airline)
KITINERARY_MAKE_PROPERTY(Airline, QString, name, setName)
KITINERARY_MAKE_PROPERTY(Airline, QString, iataCode, setIataCode)


class FlightPrivate : public QSharedData
{
public:
    QString flightNumber;
    Airline airline;
    Airport departureAirport;
    QDateTime departureTime;
    Airport arrivalAirport;
    QDateTime arrivalTime;
    QDateTime boardingTime;
    QString departureGate;
    QDate departureDay;
};

KITINERARY_MAKE_SIMPLE_CLASS(Flight)
KITINERARY_MAKE_PROPERTY(Flight, QString, flightNumber, setFlightNumber)
KITINERARY_MAKE_PROPERTY(Flight, Airline, airline, setAirline)
KITINERARY_MAKE_PROPERTY(Flight, Airport, departureAirport, setDepartureAirport)
KITINERARY_MAKE_PROPERTY(Flight, QDateTime, departureTime, setDepartureTime)
KITINERARY_MAKE_PROPERTY(Flight, Airport, arrivalAirport, setArrivalAirport)
KITINERARY_MAKE_PROPERTY(Flight, QDateTime, arrivalTime, setArrivalTime)
KITINERARY_MAKE_PROPERTY(Flight, QDateTime, boardingTime, setBoardingTime)
KITINERARY_MAKE_PROPERTY(Flight, QString, departureGate, setDepartureGate)

QDate Flight::departureDay() const
{
    if (d->departureDay.isValid()) {
        return d->departureDay;
    }
    if (d->departureTime.isValid()) {
        return d->departureTime.date();
    }
    if (d->boardingTime.isValid()) {
        return d->boardingTime.date();
    }
    return {};
}

void Flight::setDepartureDay(const QDate &value)
{
    d.detach();
    d->departureDay = value;
}

QString Flight::departureTimeLocalized() const
{
    K_D(const Flight);
    return localizedDateTime(d->departureTime);
}

QString Flight::arrivalTimeLocalized() const
{
    K_D(const Flight);
    return localizedDateTime(d->arrivalTime);
}

QString Flight::boardingTimeLocalized() const
{
    K_D(const Flight);
    auto s = QLocale().toString(d->boardingTime.time(), QLocale::ShortFormat);
    if (d->boardingTime.timeSpec() == Qt::TimeZone || d->boardingTime.timeSpec() == Qt::OffsetFromUTC) {
        s += QLatin1Char(' ') + d->boardingTime.timeZone().abbreviation(d->boardingTime);
    }
    return s;
}

}

#include "moc_flight.cpp"
