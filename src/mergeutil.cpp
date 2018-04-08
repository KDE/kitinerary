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
#include <QVariant>

using namespace KItinerary;

static bool isSameTrainTrip(const TrainTrip &lhs, const TrainTrip &rhs);
static bool isSameBusTrip(const BusTrip &lhs, const BusTrip &rhs);
static bool isSameLodingBusiness(const LodgingBusiness &lhs, const LodgingBusiness &rhs);

bool MergeUtil::isSameReservation(const QVariant& lhs, const QVariant& rhs)
{
    if (lhs.isNull() || rhs.isNull()) {
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

    // for all: underName either matches or is not set
    const auto lhsUN = JsonLdDocument::readProperty(lhs, "underName");
    const auto rhsUN = JsonLdDocument::readProperty(rhs, "underName");
    return lhsUN.isNull() || rhsUN.isNull() || isSamePerson(lhsUN.value<Person>(), rhsUN.value<Person>());
}

bool MergeUtil::isSameFlight(const Flight& lhs, const Flight& rhs)
{
    if (lhs.flightNumber().isEmpty() || rhs.flightNumber().isEmpty()) {
        return false;
    }

    return lhs.flightNumber() == rhs.flightNumber() && lhs.airline().iataCode() == rhs.airline().iataCode() && lhs.departureDay() == rhs.departureDay();
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

bool MergeUtil::isSamePerson(const Person& lhs, const Person& rhs)
{
    // TODO: extend this once Person has familiy and given name fields
    return lhs.name().compare(rhs.name(), Qt::CaseInsensitive) == 0;
}
