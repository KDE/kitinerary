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
#include "logging.h"
#include "stringutil.h"

#include <KItinerary/BusTrip>
#include <KItinerary/Event>
#include <KItinerary/Flight>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/RentalCar>
#include <KItinerary/Organization>
#include <KItinerary/Place>
#include <KItinerary/Person>
#include <KItinerary/Reservation>
#include <KItinerary/Taxi>
#include <KItinerary/TrainTrip>
#include <KItinerary/Visit>

#include <QDate>
#include <QDebug>
#include <QMetaObject>
#include <QMetaProperty>
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
static bool equalAndPresent(const QDateTime &lhs, const QDateTime &rhs)
{
    return lhs.isValid() && lhs == rhs;
}

/* Checks that @p lhs and @p rhs are not non-equal if both values are set. */
static bool conflictIfPresent(const QString &lhs, const QString &rhs, Qt::CaseSensitivity caseSensitive = Qt::CaseSensitive)
{
    return !lhs.isEmpty() && !rhs.isEmpty() && lhs.compare(rhs, caseSensitive) != 0;
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
static bool isSameTouristAttractionVisit(const TouristAttractionVisit &lhs, const TouristAttractionVisit &rhs);
static bool isSameTouristAttraction(const TouristAttraction &lhs, const TouristAttraction &rhs);
static bool isSameEvent(const Event &lhs, const Event &rhs);
static bool isSameRentalCar(const RentalCar &lhs, const RentalCar &rhs);
static bool isSameTaxiTrip(const Taxi &lhs, const Taxi &rhs);

bool MergeUtil::isSame(const QVariant& lhs, const QVariant& rhs)
{
    if (lhs.isNull() || rhs.isNull()) {
        return false;
    }
    if (lhs.userType() != rhs.userType()) {
        return false;
    }

    // for all reservations check underName
    if (JsonLd::canConvert<Reservation>(lhs)) {
        // for all: underName either matches or is not set
        const auto lhsUN = JsonLd::convert<Reservation>(lhs).underName().value<Person>();
        const auto rhsUN = JsonLd::convert<Reservation>(rhs).underName().value<Person>();
        if (!lhsUN.name().isEmpty() && !rhsUN.name().isEmpty() &&  !isSamePerson(lhsUN, rhsUN)) {
            return false;
        }
    }

    // flight: booking ref, flight number and departure day match
    if (JsonLd::isA<FlightReservation>(lhs)) {
        const auto lhsRes = lhs.value<FlightReservation>();
        const auto rhsRes = rhs.value<FlightReservation>();
        if (conflictIfPresent(lhsRes.reservationNumber(), rhsRes.reservationNumber()) || conflictIfPresent(lhsRes.passengerSequenceNumber(), rhsRes.passengerSequenceNumber())) {
            return false;
        }
        return isSame(lhsRes.reservationFor(), rhsRes.reservationFor());
    }
    if (JsonLd::isA<Flight>(lhs)) {
        const auto lhsFlight = lhs.value<Flight>();
        const auto rhsFlight = rhs.value<Flight>();
        return isSameFlight(lhsFlight, rhsFlight);
    }

    // train: booking ref, train number and depature day match
    if (JsonLd::isA<TrainReservation>(lhs)) {
        const auto lhsRes = lhs.value<TrainReservation>();
        const auto rhsRes = rhs.value<TrainReservation>();
        if (lhsRes.reservationNumber() != rhsRes.reservationNumber()) {
            return false;
        }
        return isSame(lhsRes.reservationFor(), rhsRes.reservationFor());
    }
    if (JsonLd::isA<TrainTrip>(lhs)) {
        const auto lhsTrip = lhs.value<TrainTrip>();
        const auto rhsTrip = rhs.value<TrainTrip>();
        return isSameTrainTrip(lhsTrip, rhsTrip);
    }

    // bus: booking ref, number and depature time match
    if (JsonLd::isA<BusReservation>(lhs)) {
        const auto lhsRes = lhs.value<BusReservation>();
        const auto rhsRes = rhs.value<BusReservation>();
        if (lhsRes.reservationNumber() != rhsRes.reservationNumber()) {
            return false;
        }
        return isSame(lhsRes.reservationFor(), rhsRes.reservationFor());
    }
    if (JsonLd::isA<BusTrip>(lhs)) {
        const auto lhsTrip = lhs.value<BusTrip>();
        const auto rhsTrip = rhs.value<BusTrip>();
        return isSameBusTrip(lhsTrip, rhsTrip);
    }

    // hotel: booking ref, checkin day, name match
    if (JsonLd::isA<LodgingReservation>(lhs)) {
        const auto lhsRes = lhs.value<LodgingReservation>();
        const auto rhsRes = rhs.value<LodgingReservation>();
        if (lhsRes.reservationNumber() != rhsRes.reservationNumber()) {
            return false;
        }
        return isSame(lhsRes.reservationFor(), rhsRes.reservationFor()) && lhsRes.checkinTime().date() == rhsRes.checkinTime().date();
    }
    if (JsonLd::isA<LodgingBusiness>(lhs)) {
        const auto lhsHotel = lhs.value<LodgingBusiness>();
        const auto rhsHotel = rhs.value<LodgingBusiness>();
        return isSameLodingBusiness(lhsHotel, rhsHotel);
    }

    // Rental Car
    if (JsonLd::isA<RentalCarReservation>(lhs)) {
        const auto lhsRes = lhs.value<RentalCarReservation>();
        const auto rhsRes = rhs.value<RentalCarReservation>();
        if (lhsRes.reservationNumber() != rhsRes.reservationNumber()) {
            return false;
        }
        return isSame(lhsRes.reservationFor(), rhsRes.reservationFor()) && lhsRes.pickupTime().date() == rhsRes.pickupTime().date();
    }
    if (JsonLd::isA<RentalCar>(lhs)) {
        const auto lhsEv = lhs.value<RentalCar>();
        const auto rhsEv = rhs.value<RentalCar>();
        return isSameRentalCar(lhsEv, rhsEv);
    }

    // Taxi
    if (JsonLd::isA<TaxiReservation>(lhs)) {
        const auto lhsRes = lhs.value<TaxiReservation>();
        const auto rhsRes = rhs.value<TaxiReservation>();
        if (lhsRes.reservationNumber() != rhsRes.reservationNumber()) {
            return false;
        }
        return isSame(lhsRes.reservationFor(), rhsRes.reservationFor()) && lhsRes.pickupTime().date() == rhsRes.pickupTime().date();
    }
    if (JsonLd::isA<Taxi>(lhs)) {
        const auto lhsEv = lhs.value<Taxi>();
        const auto rhsEv = rhs.value<Taxi>();
        return isSameTaxiTrip(lhsEv, rhsEv);
    }

    // restaurant reservation: same restaurant, same booking ref, same day
    if (JsonLd::isA<FoodEstablishmentReservation>(lhs)) {
        const auto lhsRes = lhs.value<FoodEstablishmentReservation>();
        const auto rhsRes = rhs.value<FoodEstablishmentReservation>();
        if (lhsRes.reservationNumber() != rhsRes.reservationNumber()) {
            return false;
        }
        auto endTime = rhsRes.endTime();
        if (!endTime.isValid()) {
            endTime = QDateTime(rhsRes.startTime().date(), QTime(23, 59, 59));
        }

        return isSame(lhsRes.reservationFor(), rhsRes.reservationFor()) && lhsRes.startTime().date() == endTime.date();
    }
    if (JsonLd::isA<FoodEstablishment>(lhs)) {
        const auto lhsRestaurant = lhs.value<FoodEstablishment>();
        const auto rhsRestaurant = rhs.value<FoodEstablishment>();
        return isSameFoodEstablishment(lhsRestaurant, rhsRestaurant);
    }

    // event reservation
    if (JsonLd::isA<EventReservation>(lhs)) {
        const auto lhsRes = lhs.value<EventReservation>();
        const auto rhsRes = rhs.value<EventReservation>();
        if (lhsRes.reservationNumber() != rhsRes.reservationNumber()) {
            return false;
        }
        return isSame(lhsRes.reservationFor(), rhsRes.reservationFor());
    }
    if (JsonLd::isA<Event>(lhs)) {
        const auto lhsEv = lhs.value<Event>();
        const auto rhsEv = rhs.value<Event>();
        return isSameEvent(lhsEv, rhsEv);
    }

    // tourist attraction visit
    if (JsonLd::isA<TouristAttractionVisit>(lhs)) {
        const auto l = lhs.value<TouristAttractionVisit>();
        const auto r = rhs.value<TouristAttractionVisit>();
        return isSameTouristAttractionVisit(l, r);
    }

    return true;
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

static bool isSameTouristAttractionVisit(const TouristAttractionVisit &lhs, const TouristAttractionVisit &rhs)
{
    return lhs.arrivalTime() == rhs.arrivalTime() && isSameTouristAttraction(lhs.touristAttraction(), rhs.touristAttraction());
}

static bool isSameTouristAttraction(const TouristAttraction &lhs, const TouristAttraction &rhs)
{
    return lhs.name() == rhs.name();
}

// compute the "difference" between @p lhs and @p rhs
static QString diffString(const QString &lhs, const QString &rhs)
{
    QString diff;
    // this is just a basic linear-time heuristic, this would need to be more something like
    // the Levenstein Distance algorithm
    for (int i = 0, j = 0; i < lhs.size() || j < rhs.size();) {
        if (i < lhs.size() && j < rhs.size() && StringUtil::normalize(lhs[i]) == StringUtil::normalize(rhs[j])) {
            ++i;
            ++j;
            continue;
        }
        if ((j < rhs.size() && (lhs.size() < rhs.size() || (lhs.size() == rhs.size() && j < i))) || i == lhs.size()) {
            diff += rhs[j];
            ++j;
        } else {
            diff += lhs[i];
            ++i;
        }
    }
    return diff.trimmed();
}

static bool isNameEqualish(const QString &lhs, const QString &rhs)
{
    if (lhs.isEmpty() || rhs.isEmpty()) {
        return false;
    }

    auto diff = diffString(lhs, rhs).toUpper();

    // remove honoric prefixes from the diff, in case the previous check didn't catch that
    diff.remove(QLatin1String("MRS"));
    diff.remove(QLatin1String("MR"));
    diff.remove(QLatin1String("MS"));

    // if there's letters in the diff, we assume this is different
    for (const auto c : diff) {
        if (c.isLetter()) {
            return false;
        }
    }

    return true;
}

bool MergeUtil::isSamePerson(const Person& lhs, const Person& rhs)
{
    return isNameEqualish(lhs.name(), rhs.name()) ||
        (isNameEqualish(lhs.givenName(), rhs.givenName()) && isNameEqualish(lhs.familyName(), rhs.familyName()));
}

static bool isSameEvent(const Event &lhs, const Event &rhs)
{
    return equalAndPresent(lhs.name(), rhs.name())
        && equalAndPresent(lhs.startDate(), rhs.startDate());
}

static bool isSameRentalCar(const RentalCar &lhs, const RentalCar &rhs)
{
    return lhs.name() == rhs.name();
}

static bool isSameTaxiTrip(const Taxi &lhs, const Taxi &rhs)
{
    //TODO verify
    return lhs.name() == rhs.name();
}


static Airline mergeValue(const Airline &lhs, const Airline &rhs)
{
    auto a = JsonLdDocument::apply(lhs, rhs).value<Airline>();
    // prefer the more detailed name
    if (a.name().size() < lhs.name().size()) {
        a.setName(lhs.name());
    }
    return a;
}

static QDateTime mergeValue(const QDateTime &lhs, const QDateTime &rhs)
{
    // prefer value with timezone
    return lhs.isValid() && lhs.timeSpec() == Qt::TimeZone && rhs.timeSpec() != Qt::TimeZone ? lhs : rhs;
}

QVariant MergeUtil::merge(const QVariant &lhs, const QVariant &rhs)
{
    if (rhs.isNull()) {
        return lhs;
    }
    if (lhs.isNull()) {
        return rhs;
    }
    if (lhs.userType() != rhs.userType()) {
        qCWarning(Log) << "type mismatch during merging:" << lhs << rhs;
        return {};
    }

    auto res = lhs;
    const auto mo = QMetaType(res.userType()).metaObject();
    for (int i = 0; i < mo->propertyCount(); ++i) {
        const auto prop = mo->property(i);
        if (!prop.isStored()) {
            continue;
        }

        auto lv = prop.readOnGadget(lhs.constData());
        auto rv = prop.readOnGadget(rhs.constData());
        auto mt = rv.userType();

        if (mt == qMetaTypeId<Airline>()) {
            rv = mergeValue(lv.value<Airline>(), rv.value<Airline>());
        } else if (mt == qMetaTypeId<QDateTime>()) {
            rv = mergeValue(lv.toDateTime(), rv.toDateTime());
        } else if (QMetaType(mt).metaObject()) {
            rv = merge(prop.readOnGadget(lhs.constData()), rv);
        }

        if (!rv.isNull()) {
            prop.writeOnGadget(res.data(), rv);
        }
    }

    return res;
}
