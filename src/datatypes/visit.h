/*
    Copyright (C) 2018 Luca Beltrame <lbeltrame@kde.org>

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
