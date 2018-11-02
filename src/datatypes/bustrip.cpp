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

#include "bustrip.h"
#include "datatypes_p.h"

#include <QDateTime>

using namespace KItinerary;

namespace KItinerary {

class BusTripPrivate : public QSharedData
{
public:
    QString arrivalPlatform;
    BusStation arrivalBusStop;
    QDateTime arrivalTime;
    QString departurePlatform;
    BusStation departureBusStop;
    QDateTime departureTime;
    QString busName;
    QString busNumber;
    Organization provider;
};

KITINERARY_MAKE_SIMPLE_CLASS(BusTrip)
KITINERARY_MAKE_PROPERTY(BusTrip, QString, arrivalPlatform, setArrivalPlatform)
KITINERARY_MAKE_PROPERTY(BusTrip, BusStation, arrivalBusStop, setArrivalBusStop)
KITINERARY_MAKE_PROPERTY(BusTrip, QDateTime, arrivalTime, setArrivalTime)
KITINERARY_MAKE_PROPERTY(BusTrip, QString, departurePlatform, setDeparturePlatform)
KITINERARY_MAKE_PROPERTY(BusTrip, BusStation, departureBusStop, setDepartureBusStop)
KITINERARY_MAKE_PROPERTY(BusTrip, QDateTime, departureTime, setDepartureTime)
KITINERARY_MAKE_PROPERTY(BusTrip, QString, busName, setBusName)
KITINERARY_MAKE_PROPERTY(BusTrip, QString, busNumber, setBusNumber)
KITINERARY_MAKE_PROPERTY(BusTrip, Organization, provider, setProvider)
KITINERARY_MAKE_OPERATOR(BusTrip)

}

#include "moc_bustrip.cpp"
