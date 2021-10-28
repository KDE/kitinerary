/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mergeutil.h"
#include "logging.h"
#include "compare-logging.h"
#include "stringutil.h"
#include "sortutil.h"

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
#include <KItinerary/Ticket>
#include <KItinerary/TrainTrip>
#include <KItinerary/Visit>

#include <QDate>
#include <QDebug>
#include <QMetaObject>
#include <QMetaProperty>
#include <QVariant>

#include <cmath>

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
static bool isMinimalCancelationFor(const QVariant &r, const Reservation &cancel);

bool MergeUtil::isSame(const QVariant& lhs, const QVariant& rhs)
{
    if (lhs.isNull() || rhs.isNull()) {
        return false;
    }
    if (lhs.userType() != rhs.userType()) {
        return false;
    }

    // for all reservations check underName and ticket
    if (JsonLd::canConvert<Reservation>(lhs)) {
        const auto lhsRes = JsonLd::convert<Reservation>(lhs);
        const auto rhsRes = JsonLd::convert<Reservation>(rhs);

        // for all: underName either matches or is not set
        const auto lhsUN = lhsRes.underName().value<Person>();
        const auto rhsUN = rhsRes.underName().value<Person>();
        if (!lhsUN.name().isEmpty() && !rhsUN.name().isEmpty() &&  !isSamePerson(lhsUN, rhsUN)) {
            return false;
        }

        const auto lhsTicket = lhsRes.reservedTicket().value<Ticket>();
        const auto rhsTicket = rhsRes.reservedTicket().value<Ticket>();
        if (conflictIfPresent(lhsTicket.ticketedSeat().seatNumber(), rhsTicket.ticketedSeat().seatNumber(), Qt::CaseInsensitive)) {
            return false;
        }
        // flight ticket tokens (IATA BCBP) can differ, so we need to compare the relevant bits in them manually
        // this however happens automatically as they are unpacked to other fields by post-processing
        // so we can simply skip this here for flights
        if (!JsonLd::isA<FlightReservation>(lhs) && conflictIfPresent(lhsTicket.ticketTokenData(), rhsTicket.ticketTokenData())) {
            return false;
        }

        // one side is a minimal cancelation, matches the reservation number and has a plausible modification time
        // in this case don't bother comparing content (which will fail), we accept this directly
        if (isMinimalCancelationFor(lhs, rhsRes) || isMinimalCancelationFor(rhs, lhsRes)) {
            return true;
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
        if (conflictIfPresent(lhsRes.reservationNumber(), rhsRes.reservationNumber())) {
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

// see kpublictrainport, line.cpp
template <typename Iter>
static bool isSameLineName(const Iter &lBegin, const Iter &lEnd, const Iter &rBegin, const Iter &rEnd)
{
    auto lIt = lBegin;
    auto rIt = rBegin;
    while (lIt != lEnd && rIt != rEnd) {
        // ignore spaces etc.
        if (!(*lIt).isLetter() && !(*lIt).isDigit()) {
            ++lIt;
            continue;
        }
        if (!(*rIt).isLetter() && !(*rIt).isDigit()) {
            ++rIt;
            continue;
        }

        if ((*lIt).toCaseFolded() != (*rIt).toCaseFolded()) {
            return false;
        }

        ++lIt;
        ++rIt;
    }

    if (lIt == lEnd && rIt == rEnd) { // both inputs fully consumed, and no mismatch found
        return true;
    }

    // one input is prefix of the other, that is ok if there's a separator
    return (lIt != lEnd && (*lIt).isSpace()) || (rIt != rEnd && (*rIt).isSpace());
}

static bool isSameLineName(const QString &lhs, const QString &rhs)
{
    return isSameLineName(lhs.begin(), lhs.end(), rhs.begin(), rhs.end())
        || isSameLineName(lhs.rbegin(), lhs.rend(), rhs.rbegin(), rhs.rend());
}

static bool isSameTrainTrip(const TrainTrip &lhs, const TrainTrip &rhs)
{
    if (lhs.departureDay() != rhs.departureDay()) {
        return false;
    }

    // for unbound tickets, comparing the line number below wont help
    // so we have to use the slightly less robust location comparisson
    if (!lhs.departureTime().isValid() && !rhs.departureTime().isValid()) {
        qCDebug(CompareLog) << "unbound trip" << lhs.departureStation().name() << rhs.departureStation().name() << lhs.arrivalStation().name() << rhs.arrivalStation().name();
        return lhs.departureStation().name() == rhs.departureStation().name() && lhs.arrivalStation().name() == rhs.arrivalStation().name();
    } else if (!equalAndPresent(lhs.departureTime(), rhs.departureTime())) {
        return false;
    }

    if (lhs.trainNumber().isEmpty() || rhs.trainNumber().isEmpty()) {
        qCDebug(CompareLog) << "missing train number" << lhs.trainNumber() << rhs.trainNumber();
        return false;
    }

    const auto isSameLine = isSameLineName(lhs.trainNumber(), rhs.trainNumber());
    qCDebug(CompareLog) << "left:" << lhs.trainName() << lhs.trainNumber() << lhs.departureTime();
    qCDebug(CompareLog) << "right:" << rhs.trainName() << rhs.trainNumber() << rhs.departureTime();
    qCDebug(CompareLog) << "same line:" << isSameLine;
    return !conflictIfPresent(lhs.trainName(),rhs.trainName()) && isSameLine && lhs.departureTime().date() == rhs.departureTime().date();
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

static bool containsNonAscii(const QString &s)
{
    for (const auto c : s) {
        if (c.row() != 0 || c.cell() > 127) {
            return true;
        }
    }

    return false;
}

static bool isMixedCase(const QString &s)
{
    const auto upperCount = std::count_if(s.begin(), s.end(), [](auto c) { return c.isUpper(); });
    return upperCount != s.size() && upperCount != 0;
}

/** Assuming both sides refer to the same thing, this tries to find the "better" one. */
static QString mergeString(const QString &lhs, const QString &rhs)
{
    // prefer the one that exists at all
    if (lhs.isEmpty()) {
        return rhs;
    }
    if (rhs.isEmpty()) {
        return lhs;
    }

    // prefer Unicode over ASCII normalization
    const auto lhsNonAscii = containsNonAscii(lhs);
    const auto rhsNonAscii = containsNonAscii(rhs);
    if (lhsNonAscii && !rhsNonAscii) {
        return lhs;
    }
    if (!lhsNonAscii && rhsNonAscii) {
        return rhs;
    }

    // prefer better casing
    const auto lhsMixedCase = isMixedCase(lhs);
    const auto rhsMixedCase = isMixedCase(rhs);
    if (lhsMixedCase && !rhsMixedCase) {
        return lhs;
    }
    if (!lhsMixedCase && rhsMixedCase) {
        return rhs;
    }

    // prefer longer == more detailed version
    if (rhs.size() < lhs.size()) {
        return lhs;
    }
    return rhs;
}

static Airline mergeValue(const Airline &lhs, const Airline &rhs)
{
    auto a = JsonLdDocument::apply(lhs, rhs).value<Airline>();
    a.setName(mergeString(lhs.name(), rhs.name()));
    return a;
}

static QDateTime mergeValue(const QDateTime &lhs, const QDateTime &rhs)
{
    // prefer value with timezone
    return lhs.isValid() && lhs.timeSpec() == Qt::TimeZone && rhs.timeSpec() != Qt::TimeZone ? lhs : rhs;
}

static Person mergeValue(const Person &lhs, const Person &rhs)
{
    auto p = JsonLdDocument::apply(lhs, rhs).value<Person>();
    p.setFamilyName(mergeString(lhs.familyName(), rhs.familyName()));
    p.setGivenName(mergeString(lhs.givenName(), rhs.givenName()));
    p.setName(mergeString(lhs.name(), rhs.name()));
    return p;
}

static Ticket mergeValue(const Ticket &lhs, const Ticket &rhs)
{
    auto t = JsonLdDocument::apply(lhs, rhs).value<Ticket>();
    // prefer barcode ticket tokens over URLs
    if (t.ticketTokenType() == Ticket::Url && lhs.ticketTokenType() != Ticket::Url && lhs.ticketTokenType() != Ticket::Unknown) {
        t.setTicketToken(lhs.ticketToken());
    }
    return t;
}

static bool checkValueIsNull(const QVariant &v)
{
    if (v.type() == qMetaTypeId<float>()) {
        return std::isnan(v.toFloat());
    }
    return v.isNull();
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

    // prefer the element with the newer mtime, if we have that information
    if (JsonLd::canConvert<Reservation>(lhs) && JsonLd::canConvert<Reservation>(rhs)) {
        const auto lhsDt = JsonLd::convert<Reservation>(lhs).modifiedTime();
        const auto rhsDt = JsonLd::convert<Reservation>(rhs).modifiedTime();
        if (lhsDt.isValid() && rhsDt.isValid() && rhsDt < lhsDt) {
            return MergeUtil::merge(rhs, lhs);
        }
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
        } else if (mt == qMetaTypeId<Person>()) {
            rv = mergeValue(lv.value<Person>(), rv.value<Person>());
        } else if (mt == qMetaTypeId<QDateTime>()) {
            rv = mergeValue(lv.toDateTime(), rv.toDateTime());
        } else if (mt == qMetaTypeId<Ticket>()) {
            rv = mergeValue(lv.value<Ticket>(), rv.value<Ticket>());
        } else if (QMetaType(mt).metaObject()) {
            rv = merge(prop.readOnGadget(lhs.constData()), rv);
        }

        if (!checkValueIsNull(rv)) {
            prop.writeOnGadget(res.data(), rv);
        }
    }

    return res;
}

bool isMinimalCancelationFor(const QVariant &r, const Reservation &cancel)
{
    const auto res = JsonLd::convert<Reservation>(r);
    if (res.reservationStatus() == Reservation::ReservationCancelled || cancel.reservationStatus() != Reservation::ReservationCancelled) {
        return false;
    }
    if (!equalAndPresent(res.reservationNumber(), cancel.reservationNumber())) {
        return false;
    }
    if (!cancel.modifiedTime().isValid() || !cancel.reservationFor().isNull()) {
        return false;
    }
    return SortUtil::startDateTime(r) > cancel.modifiedTime();
}
