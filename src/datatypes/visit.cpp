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

#include "visit.h"
#include "place.h"
#include "datatypes_p.h"

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

QString TouristAttractionVisit::departureTimeLocalized() const
{
    K_D(const TouristAttractionVisit);
    return QLocale().toString(d->departureTime, QLocale::ShortFormat);
}

QString TouristAttractionVisit::arrivalTimeLocalized() const
{
    K_D(const TouristAttractionVisit);
    return QLocale().toString(d->arrivalTime, QLocale::ShortFormat);
}

}

#include "moc_visit.cpp"
