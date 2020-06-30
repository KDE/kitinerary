/*
    SPDX-FileCopyrightText: 2018 Luca Beltrame <lbeltrame@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "visit.h"
#include "place.h"
#include "datatypes_p.h"

#include <QDateTime>

using namespace KItinerary;

namespace KItinerary {

class TouristAttractionVisitPrivate: public QSharedData {

public:
    TouristAttraction touristAttraction;
    QDateTime arrivalTime;
    QDateTime departureTime;
};

KITINERARY_MAKE_SIMPLE_CLASS(TouristAttractionVisit)
KITINERARY_MAKE_PROPERTY(TouristAttractionVisit, TouristAttraction, touristAttraction, setTouristAttraction)
KITINERARY_MAKE_PROPERTY(TouristAttractionVisit, QDateTime, arrivalTime, setArrivalTime)
KITINERARY_MAKE_PROPERTY(TouristAttractionVisit, QDateTime, departureTime, setDepartureTime)
KITINERARY_MAKE_OPERATOR(TouristAttractionVisit)

}

#include "moc_visit.cpp"
