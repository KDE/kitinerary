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
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "locationutil.h"

#include <KItinerary/BusTrip>
#include <KItinerary/Event>
#include <KItinerary/Flight>
#include <KItinerary/Place>
#include <KItinerary/Reservation>
#include <KItinerary/TrainTrip>
#include <KItinerary/Visit>

#include <QDebug>

#include <cmath>

using namespace KItinerary;

bool LocationUtil::isLocationChange(const QVariant &res)
{
    if (JsonLd::isA<RentalCarReservation>(res)) {
        const auto pickup = departureLocation(res);
        const auto dropoff = arrivalLocation(res);
        if (dropoff.value<Place>().name().isEmpty()) {
            return false;
        }
        return !isSameLocation(pickup, dropoff);
    }
    return JsonLd::isA<FlightReservation>(res) || JsonLd::isA<TrainReservation>(res) || JsonLd::isA<BusReservation>(res);
}

QVariant LocationUtil::arrivalLocation(const QVariant &res)
{
    if (JsonLd::isA<FlightReservation>(res)) {
        return res.value<FlightReservation>().reservationFor().value<Flight>().arrivalAirport();
    }
    if (JsonLd::isA<TrainReservation>(res)) {
        return res.value<TrainReservation>().reservationFor().value<TrainTrip>().arrivalStation();
    }
    if (JsonLd::isA<BusReservation>(res)) {
        return res.value<BusReservation>().reservationFor().value<BusTrip>().arrivalBusStop();
    }
    if (JsonLd::isA<RentalCarReservation>(res)) {
        return res.value<RentalCarReservation>().dropoffLocation();
    }
    return {};
}

QVariant LocationUtil::departureLocation(const QVariant &res)
{
    if (JsonLd::isA<FlightReservation>(res)) {
        return res.value<FlightReservation>().reservationFor().value<Flight>().departureAirport();
    }
    if (JsonLd::isA<TrainReservation>(res)) {
        return res.value<TrainReservation>().reservationFor().value<TrainTrip>().departureStation();
    }
    if (JsonLd::isA<BusReservation>(res)) {
        return res.value<BusReservation>().reservationFor().value<BusTrip>().departureBusStop();
    }
    if (JsonLd::isA<RentalCarReservation>(res)) {
        return res.value<RentalCarReservation>().pickupLocation();
    }
    return {};
}

QVariant LocationUtil::location(const QVariant &res)
{
    if (JsonLd::isA<LodgingReservation>(res)) {
        return res.value<LodgingReservation>().reservationFor();
    }
    if (JsonLd::isA<FoodEstablishmentReservation>(res)) {
        return res.value<FoodEstablishmentReservation>().reservationFor();
    }
    if (JsonLd::isA<TouristAttractionVisit>(res)) {
        return res.value<TouristAttractionVisit>().touristAttraction();
    }
    if (JsonLd::isA<EventReservation>(res)) {
        return res.value<EventReservation>().reservationFor().value<Event>().location();
    }
    if (JsonLd::isA<RentalCarReservation>(res)) {
        return res.value<RentalCarReservation>().pickupLocation();
    }

    return {};
}

GeoCoordinates LocationUtil::geo(const QVariant &location)
{
    if (JsonLd::canConvert<Place>(location)) {
        return JsonLd::convert<Place>(location).geo();
    }
    if (JsonLd::canConvert<Organization>(location)) {
        return JsonLd::convert<Organization>(location).geo();
    }

    return {};
}

PostalAddress LocationUtil::address(const QVariant &location)
{
    if (JsonLd::canConvert<Place>(location)) {
        return JsonLd::convert<Place>(location).address();
    }
    if (JsonLd::canConvert<Organization>(location)) {
        return JsonLd::convert<Organization>(location).address();
    }

    return {};
}

QString LocationUtil::name(const QVariant &location)
{
    if (JsonLd::isA<Airport>(location)) {
        const auto airport = location.value<Airport>();
        return airport.name().isEmpty() ? airport.iataCode() : airport.name();
    }
    if (JsonLd::canConvert<Place>(location)) {
        return JsonLd::convert<Place>(location).name();
    }
    if (JsonLd::canConvert<Organization>(location)) {
        return JsonLd::convert<Organization>(location).name();
    }

    return {};
}

// see https://en.wikipedia.org/wiki/Haversine_formula
int LocationUtil::distance(const GeoCoordinates &coord1, const GeoCoordinates &coord2)
{
    const auto degToRad = M_PI / 180.0;
    const auto earthRadius = 6371000.0; // in meters

    const auto d_lat = (coord1.latitude() - coord2.latitude()) * degToRad;
    const auto d_lon = (coord1.longitude() - coord2.longitude()) * degToRad;

    const auto a = pow(sin(d_lat / 2.0), 2) + cos(coord1.latitude() * degToRad) * cos(coord2.latitude() * degToRad) * pow(sin(d_lon / 2.0), 2);
    return 2.0 * earthRadius * atan2(sqrt(a), sqrt(1.0 - a));
}

bool LocationUtil::isSameLocation(const QVariant &lhs, const QVariant &rhs, LocationUtil::Accuracy accuracy)
{
    const auto lhsGeo = geo(lhs);
    const auto rhsGeo = geo(rhs);
    if (lhsGeo.isValid() && rhsGeo.isValid()) {
        const auto d = distance(lhsGeo, rhsGeo);
        switch (accuracy) {
            case Exact:
                return d < 100;
            case CityLevel:
                return d < 50000;
                break;
        }
        return false;
    }

    const auto lhsAddr = address(lhs);
    const auto rhsAddr = address(rhs);
    switch (accuracy) {
        case Exact:
            if (!lhsAddr.streetAddress().isEmpty() && !lhsAddr.addressLocality().isEmpty()) {
                return  lhsAddr.streetAddress() == rhsAddr.streetAddress() && lhsAddr.addressLocality() == rhsAddr.addressLocality();
            }
            break;
        case CityLevel:
            if (!lhsAddr.addressLocality().isEmpty()) {
                return lhsAddr.addressLocality() == rhsAddr.addressLocality();
            }
            break;
    }

    const auto lhsName = name(lhs);
    return !lhsName.isEmpty() && lhsName == name(rhs);
}
