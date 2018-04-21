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

#include "mergeutil.h"

#include <KItinerary/BusTrip>
#include <KItinerary/Flight>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/Organization>
#include <KItinerary/Place>
#include <KItinerary/Person>
#include <KItinerary/Reservation>
#include <KItinerary/TrainTrip>

#include <QDate>
#include <QDebug>
#include <QVariant>

using namespace KItinerary;

/* Checks that @p lhs and @p rhs are non-empty and equal. */
static bool equalAndPresent(const QString &lhs, const QString &rhs, Qt::CaseSensitivity caseSensitive = Qt::CaseSensitive)
{
    return !lhs.isEmpty() && (lhs.compare(rhs, caseSensitive) == 0);
}
static bool equalAndPresent(const QDate &lhs, const QDate &rhs)
{
    return lhs.isValid() && lhs == rhs;
}

/* Checks that @p lhs and @p rhs are not non-equal if both values are set. */
static bool conflictIfPresent(const QString &lhs, const QString &rhs)
{
    return !lhs.isEmpty() && !rhs.isEmpty() && lhs != rhs;
}
static bool conflictIfPresent(const QDateTime &lhs, const QDateTime &rhs)
{
    return lhs.isValid() && rhs.isValid() && lhs != rhs;
}

static bool isSameFlight(const Flight &lhs, const Flight &rhs);
static bool isSameTrainTrip(const TrainTrip &lhs, const TrainTrip &rhs);
static bool isSameBusTrip(const BusTrip &lhs, const BusTrip &rhs);
static bool isSameLodingBusiness(const LodgingBusiness &lhs, const LodgingBusiness &rhs);
static bool isSameFoodEstablishment(const FoodEstablishment &lhs, const FoodEstablishment &rhs);

bool MergeUtil::isSameReservation(const QVariant& lhs, const QVariant& rhs)
{
    if (lhs.isNull() || rhs.isNull() || !JsonLd::canConvert<Reservation>(lhs) || !JsonLd::canConvert<Reservation>(rhs)) {
        return false;
    }
    if (lhs.userType() != rhs.userType()) {
        return false;
    }

    // flight: booking ref, flight number and departure day match
    if (lhs.userType() == qMetaTypeId<FlightReservation>()) {
        const auto lhsRes = lhs.value<FlightReservation>();
        const auto rhsRes = rhs.value<FlightReservation>();
        if (lhsRes.reservationNumber() != rhsRes.reservationNumber() || lhsRes.reservationNumber().isEmpty()) {
            return false;
        }
        const auto lhsFlight = lhsRes.reservationFor().value<Flight>();
        const auto rhsFlight = rhsRes.reservationFor().value<Flight>();
        if (!isSameFlight(lhsFlight, rhsFlight)) {
            return false;
        }
    }

    // train: booking ref, train number and depature day match
    if (lhs.userType() == qMetaTypeId<TrainReservation>()) {
        const auto lhsRes = lhs.value<TrainReservation>();
        const auto rhsRes = rhs.value<TrainReservation>();
        if (lhsRes.reservationNumber() != rhsRes.reservationNumber()) {
            return false;
        }
        const auto lhsTrip = lhsRes.reservationFor().value<TrainTrip>();
        const auto rhsTrip = rhsRes.reservationFor().value<TrainTrip>();
        if (!isSameTrainTrip(lhsTrip, rhsTrip)) {
            return false;
        }
    }

    // bus: booking ref, number and depature time match
    if (lhs.userType() == qMetaTypeId<BusReservation>()) {
        const auto lhsRes = lhs.value<BusReservation>();
        const auto rhsRes = rhs.value<BusReservation>();
        if (lhsRes.reservationNumber() != rhsRes.reservationNumber()) {
            return false;
        }
        const auto lhsTrip = lhsRes.reservationFor().value<BusTrip>();
        const auto rhsTrip = rhsRes.reservationFor().value<BusTrip>();
        if (!isSameBusTrip(lhsTrip, rhsTrip)) {
            return false;
        }
    }

    // hotel: booking ref, checkin day, name match
    if (lhs.userType() == qMetaTypeId<LodgingReservation>()) {
        const auto lhsRes = lhs.value<LodgingReservation>();
        const auto rhsRes = rhs.value<LodgingReservation>();
        if (lhsRes.reservationNumber() != rhsRes.reservationNumber()) {
            return false;
        }
        const auto lhsHotel = lhsRes.reservationFor().value<LodgingBusiness>();
        const auto rhsHotel = rhsRes.reservationFor().value<LodgingBusiness>();
        if (!isSameLodingBusiness(lhsHotel, rhsHotel) || lhsRes.checkinTime().date() != rhsRes.checkinTime().date()) {
            return false;
        }
    }

    // restaurant reservation: sane restaurant, same booking ref, same day
    if (lhs.userType() == qMetaTypeId<FoodEstablishmentReservation>()) {
        const auto lhsRes = lhs.value<FoodEstablishmentReservation>();
        const auto rhsRes = rhs.value<FoodEstablishmentReservation>();
        if (lhsRes.reservationNumber() != rhsRes.reservationNumber()) {
            return false;
        }
        const auto lhsRestaurant = lhsRes.reservationFor().value<FoodEstablishment>();
        const auto rhsRestaurant = rhsRes.reservationFor().value<FoodEstablishment>();
        if (!isSameFoodEstablishment(lhsRestaurant, rhsRestaurant) || lhsRes.startTime().date() != rhsRes.endTime().date()) {
            return false;
        }
    }

    // for all: underName either matches or is not set
    const auto lhsUN = JsonLd::convert<Reservation>(lhs).underName();
    const auto rhsUN = JsonLd::convert<Reservation>(rhs).underName();
    return lhsUN.isNull() || rhsUN.isNull() || isSamePerson(lhsUN.value<Person>(), rhsUN.value<Person>());
}

static bool isSameFlight(const Flight& lhs, const Flight& rhs)
{
    // if there is a conflict on where this is going, or when, this is obviously not the same flight
    if (conflictIfPresent(lhs.departureAirport().iataCode(), rhs.departureAirport().iataCode()) ||
        conflictIfPresent(lhs.arrivalAirport().iataCode(), rhs.arrivalAirport().iataCode()) ||
        !equalAndPresent(lhs.departureDay(), rhs.departureDay())) {
        return false;
    }

    // same flight number and airline (on the same day) -> we assume same flight
    if (equalAndPresent(lhs.flightNumber(), rhs.flightNumber()) && equalAndPresent(lhs.airline().iataCode(), rhs.airline().iataCode())) {
        return true;
    }

    // we get here if we have matching origin/destination on the same day, but mismatching flight numbers
    // so this might be a codeshare flight
    // our caller checks for matching booking ref, so just look for a few counter-indicators here
    // (that is, if this is ever made available as standalone API, the last return should not be true)
    if (conflictIfPresent(lhs.departureTime(), rhs.departureTime())) {
        return false;
    }

    return true;
}

static bool isSameTrainTrip(const TrainTrip &lhs, const TrainTrip &rhs)
{
    if (lhs.trainNumber().isEmpty() || rhs.trainNumber().isEmpty()) {
        return false;
    }

    return lhs.trainName() == rhs.trainName() && lhs.trainNumber() == rhs.trainNumber() && lhs.departureTime().date() == rhs.departureTime().date();
}

static bool isSameBusTrip(const BusTrip &lhs, const BusTrip &rhs)
{
    if (lhs.busNumber().isEmpty() || rhs.busNumber().isEmpty()) {
        return false;
    }

    return lhs.busName() == rhs.busName() && lhs.busNumber() == rhs.busNumber() && lhs.departureTime() == rhs.departureTime();
}

static bool isSameLodingBusiness(const LodgingBusiness &lhs, const LodgingBusiness &rhs)
{
    if (lhs.name().isEmpty() || rhs.name().isEmpty()) {
        return false;
    }

    return lhs.name() == rhs.name();
}

static bool isSameFoodEstablishment(const FoodEstablishment &lhs, const FoodEstablishment &rhs)
{
    if (lhs.name().isEmpty() || rhs.name().isEmpty()) {
        return false;
    }

    return lhs.name() == rhs.name();
}

bool MergeUtil::isSamePerson(const Person& lhs, const Person& rhs)
{
    if (lhs.name().compare(rhs.name(), Qt::CaseInsensitive) == 0) {
        return true;
    }

    return equalAndPresent(lhs.familyName(), rhs.familyName(), Qt::CaseInsensitive)
        && equalAndPresent(lhs.givenName(), rhs.givenName(), Qt::CaseInsensitive);

    // TODO deal with cases where on side has the name split, and the other side only has the full name
}
