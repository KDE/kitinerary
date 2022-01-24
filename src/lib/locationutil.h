/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kitinerary_export.h"

class QString;
class QUrl;
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
/** Computes the distance between to geo coordinates in meters. */
int KITINERARY_EXPORT distance(float lat1, float lon1, float lat2, float lon2);

/** Location comparison accuracy. */
enum Accuracy {
    Exact, ///< Locations match exactly
    CityLevel, ///< Locations are in the same city
    WalkingDistance, ///< Locations are close enough together to not need transportation
};

/** Returns @c true if the given locations are the same.
 *  @param lhs The left hand side in the location condition.
 *  @param rhs The right hand side in the location condition.
 *  @param accuracy Defines how closely the locations have to match.
 */
bool KITINERARY_EXPORT isSameLocation(const QVariant &lhs, const QVariant &rhs, Accuracy accuracy = Exact);

/** Returns a geo: URI for the given location. */
QUrl KITINERARY_EXPORT geoUri(const QVariant &location);

}

}

