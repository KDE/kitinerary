/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "traintrip.h"
#include "datatypes_impl.h"
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
    QDate departureDay;
    QString trainName;
    QString trainNumber;
};

KITINERARY_MAKE_CLASS(TrainTrip)
KITINERARY_MAKE_PROPERTY(TrainTrip, QString, arrivalPlatform, setArrivalPlatform)
KITINERARY_MAKE_PROPERTY(TrainTrip, TrainStation, arrivalStation, setArrivalStation)
KITINERARY_MAKE_PROPERTY(TrainTrip, QDateTime, arrivalTime, setArrivalTime)
KITINERARY_MAKE_PROPERTY(TrainTrip, QString, departurePlatform, setDeparturePlatform)
KITINERARY_MAKE_PROPERTY(TrainTrip, TrainStation, departureStation, setDepartureStation)
KITINERARY_MAKE_PROPERTY(TrainTrip, QDateTime, departureTime, setDepartureTime)
KITINERARY_MAKE_PROPERTY(TrainTrip, Organization, provider, setProvider)
KITINERARY_MAKE_PROPERTY_OPERATOR(TrainTrip, QDate, departureDay)
KITINERARY_MAKE_PROPERTY(TrainTrip, QString, trainName, setTrainName)
KITINERARY_MAKE_PROPERTY(TrainTrip, QString, trainNumber, setTrainNumber)
KITINERARY_MAKE_OPERATOR(TrainTrip)

QDate TrainTrip::departureDay() const
{
    if (d->departureDay.isValid()) {
        return d->departureDay;
    }
    // pre-1970 dates are used as transient state when we only know the time
    if (d->departureTime.isValid() && d->departureTime.date().year() > 1970) {
        return d->departureTime.date();
    }
    return {};
}

void TrainTrip::setDepartureDay(const QDate &value)
{
    if (departureDay() != value) {
        d.detach();
        d->departureDay = value;
    }
}

}

#include "moc_traintrip.cpp"
