/*
    SPDX-FileCopyrightText: 2018 Luca Beltrame <lbeltrame@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_VISIT_H
#define KITINERARY_VISIT_H

#include "kitinerary_export.h"

#include "datatypes.h"
#include "place.h"

namespace KItinerary {

class TouristAttractionVisitPrivate;

class KITINERARY_EXPORT TouristAttractionVisit
{
    KITINERARY_GADGET(TouristAttractionVisit)
    KITINERARY_PROPERTY(KItinerary::TouristAttraction, touristAttraction, setTouristAttraction)
    KITINERARY_PROPERTY(QDateTime, arrivalTime, setArrivalTime)
    KITINERARY_PROPERTY(QDateTime, departureTime, setDepartureTime)

private:
    QExplicitlySharedDataPointer<TouristAttractionVisitPrivate> d;
};

} // namespace KItinerary

Q_DECLARE_METATYPE(KItinerary::TouristAttractionVisit)

#endif // KITINERARY_VISIT_H
