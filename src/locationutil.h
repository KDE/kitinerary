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

#ifndef KITINERARY_LOCATIONUTIL_H
#define KITINERARY_LOCATIONUTIL_H

#include "kitinerary_export.h"

class QString;
class QVariant;

namespace KItinerary {

class GeoCoordinates;
class PostalAddress;

/** Utility functions related to location information. */
namespace LocationUtil
{

/** Returns @c true if the given reservation is a location change.
 *  That is, some form of transport reservation with different departure
 *  and arrival locations.
 */
bool KITINERARY_EXPORT isLocationChange(const QVariant &res);

/** Returns the departure location of the given reservation.
 *  This assumes isLocationChange(res) == true.
 */
QVariant KITINERARY_EXPORT departureLocation(const QVariant &res);

/** Returns the arrival location of the given reservation.
 *  This assumes isLocationChange(res) == true.
 */
QVariant KITINERARY_EXPORT arrivalLocation(const QVariant &res);

/** Returns the location of a non-transport reservation.
 *  This assumes isLocationChange(res) == false.
 */
QVariant KITINERARY_EXPORT location(const QVariant &res);

/** Returns the geo coordinates of a given location. */
GeoCoordinates KITINERARY_EXPORT geo(const QVariant &location);

/** Returns the address of the given location. */
PostalAddress KITINERARY_EXPORT address(const QVariant &location);

/** Returns a description of the location. */
QString KITINERARY_EXPORT name(const QVariant &location);

/** Computes the distance between to geo coordinates in meters. */
int KITINERARY_EXPORT distance(const GeoCoordinates &coord1, const GeoCoordinates &coord2);

/** Location comparison accuracy. */
enum Accuracy {
    Exact, ///< Locations match exactly
    CityLevel ///< Locations are in the same city
};

/** Returns @c true if the given locations are the same.
 *  @param accuracy Defines how closely the locations have to match.
 */
bool KITINERARY_EXPORT isSameLocation(const QVariant &lhs, const QVariant &rhs, Accuracy accuracy = Exact);

}

}

#endif // KITINERARY_LOCATIONUTIL_H
