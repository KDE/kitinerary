/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "boattrip.h"
#include "datatypes_p.h"

#include <QDateTime>

using namespace KItinerary;

namespace KItinerary {

class BoatTripPrivate : public QSharedData
{
public:
    QString name;
    BoatTerminal arrivalBoatTerminal;
    QDateTime arrivalTime;
    BoatTerminal departureBoatTerminal;
    QDateTime departureTime;
};

KITINERARY_MAKE_SIMPLE_CLASS(BoatTrip)
KITINERARY_MAKE_PROPERTY(BoatTrip, QString, name, setName)
KITINERARY_MAKE_PROPERTY(BoatTrip, BoatTerminal, arrivalBoatTerminal, setArrivalBoatTerminal)
KITINERARY_MAKE_PROPERTY(BoatTrip, QDateTime, arrivalTime, setArrivalTime)
KITINERARY_MAKE_PROPERTY(BoatTrip, BoatTerminal, departureBoatTerminal, setDepartureBoatTerminal)
KITINERARY_MAKE_PROPERTY(BoatTrip, QDateTime, departureTime, setDepartureTime)
KITINERARY_MAKE_OPERATOR(BoatTrip)

}

#include "moc_boattrip.cpp"
