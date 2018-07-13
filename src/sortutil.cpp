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

#include "sortutil.h"

#include <KItinerary/BusTrip>
#include <KItinerary/Flight>
#include <KItinerary/Person>
#include <KItinerary/Place>
#include <KItinerary/Reservation>
#include <KItinerary/TrainTrip>
#include <KItinerary/Visit>

#include <QDateTime>

using namespace KItinerary;

QDateTime SortUtil::startDateTime(const QVariant &res)
{
    if (JsonLd::isA<FlightReservation>(res)) {
        const auto flight = res.value<FlightReservation>().reservationFor().value<Flight>();
        if (flight.departureTime().isValid()) {
            return flight.departureTime();
        }
        if (flight.boardingTime().isValid()) {
            return flight.boardingTime();
        }
        return QDateTime(flight.departureDay(), QTime(23, 59, 59));
    }
    if (JsonLd::isA<TrainReservation>(res)) {
        return res.value<TrainReservation>().reservationFor().value<TrainTrip>().departureTime();
    }
    if (JsonLd::isA<BusReservation>(res)) {
        return res.value<BusReservation>().reservationFor().value<BusTrip>().departureTime();
    }
    if (JsonLd::isA<FoodEstablishmentReservation>(res)) {
        return res.value<FoodEstablishmentReservation>().startTime();
    }
    if (JsonLd::isA<LodgingReservation>(res)) {
        const auto hotel = res.value<LodgingReservation>();
        // hotel checkin/checkout is always considered the first/last thing of the day
        return QDateTime(hotel.checkinTime().date(), QTime(23, 59, 59));
    }
    if (JsonLd::isA<TouristAttractionVisit>(res)) {
        return res.value<TouristAttractionVisit>().arrivalTime();
    }

    return {};
}

QDateTime SortUtil::endtDateTime(const QVariant &res)
{
    if (JsonLd::isA<FlightReservation>(res)) {
        const auto flight = res.value<FlightReservation>().reservationFor().value<Flight>();
        if (flight.arrivalTime().isValid()) {
            return flight.arrivalTime();
        }
        return QDateTime(flight.departureDay(), QTime(23, 59, 59));
    }
    if (JsonLd::isA<TrainReservation>(res)) {
        return res.value<TrainReservation>().reservationFor().value<TrainTrip>().arrivalTime();
    }
    if (JsonLd::isA<BusReservation>(res)) {
        return res.value<BusReservation>().reservationFor().value<BusTrip>().arrivalTime();
    }
    if (JsonLd::isA<FoodEstablishmentReservation>(res)) {
        return res.value<FoodEstablishmentReservation>().endTime();
    }
    if (JsonLd::isA<LodgingReservation>(res)) {
        const auto hotel = res.value<LodgingReservation>();
        // hotel checkin/checkout is always considered the first/last thing of the day
        return QDateTime(hotel.checkoutTime().date(), QTime(0, 0, 0));
    }
    if (JsonLd::isA<TouristAttractionVisit>(res)) {
        return res.value<TouristAttractionVisit>().departureTime();
    }

    return {};
}

bool SortUtil::isBefore(const QVariant &lhs, const QVariant &rhs)
{
    if (startDateTime(lhs) == startDateTime(rhs) && lhs.userType() == rhs.userType() && JsonLd::canConvert<Reservation>(lhs)) {
        // for multi-traveler reservations, sort by traveler name to achieve a stable result
        const auto lhsRes = JsonLd::convert<Reservation>(lhs);
        const auto rhsRes = JsonLd::convert<Reservation>(rhs);
        if (!lhsRes.underName().isNull() && !rhsRes.underName().isNull()) {
            const auto lhsUN = lhsRes.underName().value<Person>();
            const auto rhsUN = rhsRes.underName().value<Person>();
            return lhsUN.name() < rhsUN.name();
        }
    }
    return startDateTime(lhs) < startDateTime(rhs);
}
