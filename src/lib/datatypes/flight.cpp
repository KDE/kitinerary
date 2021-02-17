/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "flight.h"
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
    QString arrivalTerminal;
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
KITINERARY_MAKE_PROPERTY(Flight, QString, arrivalTerminal, setArrivalTerminal)
KITINERARY_MAKE_PROPERTY(Flight, QDateTime, boardingTime, setBoardingTime)
KITINERARY_MAKE_PROPERTY(Flight, Organization, provider, setProvider)
KITINERARY_MAKE_OPERATOR(Flight)

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
