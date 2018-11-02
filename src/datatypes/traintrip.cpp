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

#include "traintrip.h"
#include "organization.h"
#include "place.h"
#include "datatypes_p.h"

#include <QDateTime>

using namespace KItinerary;

namespace KItinerary {

class TrainTripPrivate : public QSharedData
{
public:
    QString arrivalPlatform;
    TrainStation arrivalStation;
    QDateTime arrivalTime;
    QString departurePlatform;
    TrainStation departureStation;
    Organization provider;
    QDateTime departureTime;
    QString trainName;
    QString trainNumber;
};

KITINERARY_MAKE_SIMPLE_CLASS(TrainTrip)
KITINERARY_MAKE_PROPERTY(TrainTrip, QString, arrivalPlatform, setArrivalPlatform)
KITINERARY_MAKE_PROPERTY(TrainTrip, TrainStation, arrivalStation, setArrivalStation)
KITINERARY_MAKE_PROPERTY(TrainTrip, QDateTime, arrivalTime, setArrivalTime)
KITINERARY_MAKE_PROPERTY(TrainTrip, QString, departurePlatform, setDeparturePlatform)
KITINERARY_MAKE_PROPERTY(TrainTrip, TrainStation, departureStation, setDeparatureStation)
KITINERARY_MAKE_PROPERTY(TrainTrip, QDateTime, departureTime, setDepartureTime)
KITINERARY_MAKE_PROPERTY(TrainTrip, Organization, provider, setProvider)
KITINERARY_MAKE_PROPERTY(TrainTrip, QString, trainName, setTrainName)
KITINERARY_MAKE_PROPERTY(TrainTrip, QString, trainNumber, setTrainNumber)
KITINERARY_MAKE_OPERATOR(TrainTrip)

}

#include "moc_traintrip.cpp"
