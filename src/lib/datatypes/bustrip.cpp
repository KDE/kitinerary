/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
    QDate departureDay;
    QString busName;
    QString busNumber;
    Organization provider;
};

KITINERARY_MAKE_CLASS(BusTrip)
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

QDate BusTrip::departureDay() const
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

void BusTrip::setDepartureDay(const QDate &value)
{
    if (departureDay() != value) {
        d.detach();
        d->departureDay = value;
    }
}

}

#include "moc_bustrip.cpp"
