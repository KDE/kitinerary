/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mergeutil.h"
#include "logging.h"
#include "compare-logging.h"
#include "locationutil.h"
#include "stringutil.h"
#include "sortutil.h"
#include "tickettokencomparator_p.h"

#include <KItinerary/BoatTrip>
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
#include <QRegularExpression>
#include <QTimeZone>

#include <cmath>
#include <cstring>
#include <set>

namespace KItinerary {
struct CompareFunc {
    int metaTypeId;
    std::function<bool(const QVariant &, const QVariant &)> func;
};

static bool operator<(const CompareFunc &lhs, int rhs)
{
    return lhs.metaTypeId < rhs;
}

static std::vector<CompareFunc> s_mergeCompareFuncs;
}

using namespace Qt::Literals;
using namespace KItinerary;

/* Compare times without assuming times without a timezone are in the current time zone
 * (they might be local to the destination instead).
 */
static bool dateTimeCompare(const QDateTime &lhs, const QDateTime &rhs)
{
    if (lhs == rhs) {
        return true;
    }
    if (lhs.timeSpec() == Qt::LocalTime && rhs.timeSpec() != Qt::LocalTime) {
        QDateTime dt(rhs);
        dt.setTimeZone(QTimeZone::LocalTime);
        return lhs == dt;
    }
    if (lhs.timeSpec() != Qt::LocalTime && rhs.timeSpec() == Qt::LocalTime) {
        QDateTime dt(lhs);
        dt.setTimeZone(QTimeZone::LocalTime);
        return dt == rhs;
    }
    return false;
}

/* Checks that @p lhs and @p rhs are non-empty and equal. */
static bool equalAndPresent(QStringView lhs, QStringView rhs, Qt::CaseSensitivity caseSensitive = Qt::CaseSensitive)
{
    return !lhs.isEmpty() && (lhs.compare(rhs, caseSensitive) == 0);
}
template <typename T>
static typename std::enable_if<!std::is_same_v<T, QString>, bool>::type equalAndPresent(const T &lhs, const T &rhs)
{
    return lhs.isValid() && lhs == rhs;
}
static bool equalAndPresent(const QDateTime &lhs, const QDateTime &rhs)
{
    return lhs.isValid() && dateTimeCompare(lhs, rhs);
}

/* Checks that @p lhs and @p rhs are not non-equal if both values are set. */
static bool conflictIfPresent(QStringView lhs, QStringView rhs, Qt::CaseSensitivity caseSensitive = Qt::CaseSensitive)
{
    return !lhs.isEmpty() && !rhs.isEmpty() && lhs.compare(rhs, caseSensitive) != 0;
}
template <typename T>
static typename std::enable_if<!std::is_same_v<T, QString>, bool>::type conflictIfPresent(const T &lhs, const T &rhs)
{
    return lhs.isValid() && rhs.isValid() && lhs != rhs;
}
static bool conflictIfPresent(const QDateTime &lhs, const QDateTime &rhs)
{
    return lhs.isValid() && rhs.isValid() && !dateTimeCompare(lhs, rhs);
}
static bool conflictIfPresent(const Person &lhs, const Person &rhs)
{
    return !lhs.name().isEmpty() && !rhs.name().isEmpty() && !MergeUtil::isSamePerson(lhs, rhs);
}

static bool isSameFlight(const Flight &lhs, const Flight &rhs);
static bool isSameTrainTrip(const TrainTrip &lhs, const TrainTrip &rhs);
static bool isSameBusTrip(const BusTrip &lhs, const BusTrip &rhs);
static bool isSameBoatTrip(const BoatTrip &lhs, const BoatTrip &rhs);
static bool isSameLocalBusiness(const LocalBusiness &lhs, const LocalBusiness &rhs);
static bool isSameTouristAttractionVisit(const TouristAttractionVisit &lhs, const TouristAttractionVisit &rhs);
static bool isSameTouristAttraction(const TouristAttraction &lhs, const TouristAttraction &rhs);
static bool isSameEvent(const Event &lhs, const Event &rhs);
static bool isSameRentalCar(const RentalCar &lhs, const RentalCar &rhs);
static bool isSameTaxiTrip(const Taxi &lhs, const Taxi &rhs);
static bool isSameReservation(const Reservation &lhsRes, const Reservation &rhsRes);
static bool isSameSeat(const Seat &lhsSeat, const Seat &rhsSeat);
static bool isMinimalCancelationFor(const Reservation &res, const Reservation &cancel);

bool isSameReservation(const Reservation &lhsRes, const Reservation &rhsRes)
{
    // underName either matches or is not set
    if (conflictIfPresent(lhsRes.underName().value<Person>(), rhsRes.underName().value<Person>())
     || conflictIfPresent(lhsRes.reservationNumber(), rhsRes.reservationNumber())) {
        return false;
    }

    const auto lhsTicket = lhsRes.reservedTicket().value<Ticket>();
    const auto rhsTicket = rhsRes.reservedTicket().value<Ticket>();
    if (!isSameSeat(lhsTicket.ticketedSeat(), rhsTicket.ticketedSeat())) {
        return false;
    }

    return true;
}

bool isSameSeat(const Seat &lhsSeat, const Seat &rhsSeat)
{
    // check for coach number conflicts
    if (!lhsSeat.seatSection().isEmpty() && !rhsSeat.seatSection().isEmpty()) {
        bool lhsOk = false, rhsOk = false;
        const auto lhsNum = lhsSeat.seatSection().toInt(&lhsOk);
        const auto rhsNum = rhsSeat.seatSection().toInt(&rhsOk);
        if (lhsOk && rhsOk && lhsNum > 0 && rhsNum > 0 && lhsNum != rhsNum) {
            return false;
        }
        if ((!lhsOk || !rhsOk) && lhsSeat.seatSection().compare(rhsSeat.seatSection(), Qt::CaseInsensitive) != 0) {
            return false;
        }
    }

    // check for seat number conflicts
    if (conflictIfPresent(lhsSeat.seatNumber(), rhsSeat.seatNumber(), Qt::CaseInsensitive)) {
        const QRegularExpression rx(uR"([[:space:][:punct:]])"_s);
        auto lhsSet = lhsSeat.seatNumber().toCaseFolded().split(rx, Qt::SkipEmptyParts);
        auto rhsSet = rhsSeat.seatNumber().toCaseFolded().split(rx, Qt::SkipEmptyParts);
        std::ranges::sort(lhsSet);
        std::ranges::sort(rhsSet);
        return lhsSet == rhsSet;
    }

    return true;
}

bool isSubType(const QVariant &lhs, const QVariant &rhs)
{
    const auto lhsMt = QMetaType(lhs.userType());
    const auto rhsMt = QMetaType(rhs.userType());
    const auto lhsMo = lhsMt.metaObject();
    const auto rhsMo = rhsMt.metaObject();
    // for enums/flags, this is the enclosing meta object starting with Qt6!
    if (!lhsMo || !rhsMo || (lhsMt.flags() & QMetaType::IsGadget) == 0  || (rhsMt.flags() & QMetaType::IsGadget) == 0) {
        return false;
    }
    return lhsMo->inherits(rhsMo);
}

bool MergeUtil::isSame(const QVariant& lhs, const QVariant& rhs)
{
    if (lhs.isNull() || rhs.isNull()) {
        return false;
    }
    if (!isSubType(lhs, rhs) && !isSubType(rhs, lhs)) {
        return false;
    }

    // for all reservations check underName and ticket
    if (JsonLd::canConvert<Reservation>(lhs)) {
        const auto lhsRes = JsonLd::convert<Reservation>(lhs);
        const auto rhsRes = JsonLd::convert<Reservation>(rhs);
        if (!isSameReservation(lhsRes, rhsRes)) {
            return false;
        }

        const auto lhsTicket = lhsRes.reservedTicket().value<Ticket>();
        const auto rhsTicket = rhsRes.reservedTicket().value<Ticket>();
        // flight ticket tokens (IATA BCBP) can differ, so we need to compare the relevant bits in them manually
        // this however happens automatically as they are unpacked to other fields by post-processing
        // so we can simply skip this here for flights
        // for other ticket tokens (e.g. Renfe), shorter and longer versions of the same token exist as well
        // so we look for matching prefixes here
        if (!JsonLd::isA<FlightReservation>(lhs) && !TicketTokenComparator::isSame(lhsTicket.ticketTokenData(), rhsTicket.ticketTokenData())) {
            return false;
        }

        // one side is a minimal cancellation, matches the reservation number and has a plausible modification time
        // in this case don't bother comparing content (which will fail), we accept this directly
        if (isMinimalCancelationFor(lhsRes, rhsRes) || isMinimalCancelationFor(rhsRes, lhsRes)) {
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

    // train: booking ref, train number and departure day match
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
        if (conflictIfPresent(lhsRes.reservationNumber(), rhsRes.reservationNumber())) {
            return false;
        }
        return isSame(lhsRes.reservationFor(), rhsRes.reservationFor());
    }
    if (JsonLd::isA<BusTrip>(lhs)) {
        const auto lhsTrip = lhs.value<BusTrip>();
        const auto rhsTrip = rhs.value<BusTrip>();
        return isSameBusTrip(lhsTrip, rhsTrip);
    }

    // boat
    if (JsonLd::isA<BoatReservation>(lhs)) {
        const auto lhsRes = lhs.value<BoatReservation>();
        const auto rhsRes = rhs.value<BoatReservation>();
        if (lhsRes.reservationNumber() != rhsRes.reservationNumber()) {
            return false;
        }
        return isSame(lhsRes.reservationFor(), rhsRes.reservationFor());
    }
    if (JsonLd::isA<BoatTrip>(lhs)) {
        const auto lhsTrip = lhs.value<BoatTrip>();
        const auto rhsTrip = rhs.value<BoatTrip>();
        return isSameBoatTrip(lhsTrip, rhsTrip);
    }

    // hotel: booking ref, checkin day, name match
    if (JsonLd::isA<LodgingReservation>(lhs)) {
        const auto lhsRes = lhs.value<LodgingReservation>();
        const auto rhsRes = rhs.value<LodgingReservation>();
        return isSame(lhsRes.reservationFor(), rhsRes.reservationFor()) && lhsRes.checkinTime().date() == rhsRes.checkinTime().date();
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

    // generic busniess (hotel, restaurant)
    if (JsonLd::canConvert<LocalBusiness>(lhs)) {
        const auto lhsBusiness = JsonLd::convert<LocalBusiness>(lhs);
        const auto rhsBusiness = JsonLd::convert<LocalBusiness>(rhs);
        return isSameLocalBusiness(lhsBusiness, rhsBusiness);
    }

    // event reservation
    if (JsonLd::isA<EventReservation>(lhs)) {
        const auto lhsRes = lhs.value<EventReservation>();
        const auto rhsRes = rhs.value<EventReservation>();
        if (lhsRes.reservationNumber() != rhsRes.reservationNumber()) {
            return false;
        }
        return isSame(lhsRes.reservationFor(), rhsRes.reservationFor()) ||
            // TODO replace by more general handling of incremental updates for existing elements,
            // similar to how minimal cancellations are handled above
            ((lhsRes.reservationFor().isNull() ^ rhsRes.reservationFor().isNull()) && equalAndPresent(lhsRes.reservationNumber(), rhsRes.reservationNumber()));
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

    // top-level tickets
    if (JsonLd::isA<Ticket>(lhs)) {
        const auto lhsTicket = lhs.value<Ticket>();
        const auto rhsTicket = rhs.value<Ticket>();

        if (conflictIfPresent(lhsTicket.underName(), rhsTicket.underName())
         || conflictIfPresent(lhsTicket.ticketNumber(), rhsTicket.ticketNumber())
         || conflictIfPresent(lhsTicket.name(), rhsTicket.name())
         || conflictIfPresent(lhsTicket.validFrom(), rhsTicket.validFrom())
         || !TicketTokenComparator::isSame(lhsTicket.ticketTokenData(), rhsTicket.ticketTokenData())
        ) {
            return false;
        }
    }

    // program memberships
    if (JsonLd::isA<ProgramMembership>(lhs)) {
        const auto lhsPM = lhs.value<ProgramMembership>();
        const auto rhsPM = rhs.value<ProgramMembership>();

        if (conflictIfPresent(lhsPM.member(), rhsPM.member())
         || conflictIfPresent(lhsPM.programName(), rhsPM.programName())
         || conflictIfPresent(lhsPM.membershipNumber(), rhsPM.membershipNumber())
         || conflictIfPresent(lhsPM.validFrom(), rhsPM.validFrom())
         || conflictIfPresent(lhsPM.validUntil(), rhsPM.validUntil())
         || !TicketTokenComparator::isSame(lhsPM.tokenData(), rhsPM.tokenData())
        ) {
            return false;
        }
    }

    if (lhs.userType() != rhs.userType()) {
        return false;
    }

    // custom comparators
    const auto it = std::lower_bound(s_mergeCompareFuncs.begin(), s_mergeCompareFuncs.end(), lhs.userType());
    if (it != s_mergeCompareFuncs.end() && (*it).metaTypeId == lhs.userType()) {
        return (*it).func(lhs, rhs);
    }

    return true;
}

static bool isSameFlight(const Flight& lhs, const Flight& rhs)
{
    // if there is a conflict on where this is going, or when, this is obviously not the same flight
    if (conflictIfPresent(lhs.departureAirport().iataCode(), rhs.departureAirport().iataCode()) ||
        conflictIfPresent(lhs.arrivalAirport().iataCode(), rhs.arrivalAirport().iataCode()) ||
        conflictIfPresent(lhs.departureDay(), rhs.departureDay())) {
        return false;
    }

    // same flight number and airline (on the same day) -> we assume same flight
    // different airlines however could be codeshare flights
    if (equalAndPresent(lhs.airline().iataCode(), rhs.airline().iataCode())) {
        return equalAndPresent(lhs.flightNumber(), rhs.flightNumber());
    }

    // enforce same date of travel if the flight number has been inconclusive
    if (!equalAndPresent(lhs.departureDay(), rhs.departureDay())) {
        return false;
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

struct {
    const char *match;
    const char *replace;
} static constexpr const train_normalization_patterns[] = {
    { "intercityexpress", "ice" },
    { "intercity", "ic" },
};

static QString normalizeTrainName(QStringView name)
{
    QString normalized;
    normalized.reserve(name.size());
    for (const auto c : name) {
        if (!c.isDigit() && !c.isLetter()) {
            continue;
        }
        normalized.push_back(c.toCaseFolded());
    }

    for (const auto &pattern : train_normalization_patterns) {
        normalized.replace(QLatin1StringView(pattern.match), QLatin1StringView(pattern.replace));
    }

    return normalized;
}

static bool isSameTrainName(QStringView lhs, QStringView rhs)
{
    if (lhs.isEmpty() || rhs.isEmpty()) {
        return false;
    }
    const auto normalizedLhs = normalizeTrainName(lhs);
    const auto normalizedRhs = normalizeTrainName(rhs);
    return normalizedLhs.startsWith(normalizedRhs) || normalizedRhs.startsWith(normalizedLhs);
}

static bool isSameLineName(const QString &lhs, const QString &rhs)
{
    if (isSameLineName(lhs.begin(), lhs.end(), rhs.begin(), rhs.end())
        || isSameLineName(lhs.rbegin(), lhs.rend(), rhs.rbegin(), rhs.rend()))
    {
        return true;
    }

    // deal with both line and route numbers being specified
    static QRegularExpression rx(QStringLiteral(R"(^(?<type>[A-Za-z-]+) (?<line>\d+)(?: \((?<route>\d{4,})\))?$)"));
    const auto lhsMatch = rx.match(lhs);
    const auto rhsMatch = rx.match(rhs);
    if (!lhsMatch.hasMatch() || !rhsMatch.hasMatch()) {
        return false;
    }
    return isSameTrainName(lhsMatch.capturedView(u"type"), rhsMatch.capturedView(u"type")) && (
           (equalAndPresent(lhsMatch.capturedView(u"line"), rhsMatch.capturedView(u"line")) && !conflictIfPresent(lhsMatch.capturedView(u"route"), rhsMatch.capturedView(u"route")))
        || (equalAndPresent(lhsMatch.capturedView(u"line"), rhsMatch.capturedView(u"route")) && lhsMatch.capturedView(u"route").isEmpty())
        || (equalAndPresent(lhsMatch.capturedView(u"route"), rhsMatch.capturedView(u"line")) && rhsMatch.capturedView(u"route").isEmpty())
    );
}

[[nodiscard]] static bool isSameTrainType(QStringView lhs, QStringView rhs)
{
    qsizetype i = 0;
    for (; i < lhs.size() && rhs.size(); ++i) {
        if (lhs[i] != rhs[i]) {
            break;
        }
    }

    if (i == 0 || i == lhs.size() || i == rhs.size()) {
        return false;
    }

    return std::ranges::all_of(lhs.mid(i), [](QChar c) { return c.isNumber(); })
        && std::ranges::all_of(rhs.mid(i), [](QChar c) { return c.isNumber(); });
}

static bool isSameTrainTrip(const TrainTrip &lhs, const TrainTrip &rhs)
{
    if (lhs.departureDay() != rhs.departureDay()) {
        return false;
    }

    if (lhs.departureTime().isValid() && rhs.departureTime().isValid()) {
        // if we have both departure times, they have to match
        qCDebug(CompareLog) << "departure time" << lhs.departureTime() << rhs.departureTime() <<equalAndPresent(lhs.departureTime(), rhs.departureTime());
        if (!equalAndPresent(lhs.departureTime(), rhs.departureTime())) {
            return false;
        }
    } else {
        // only one departure time exists, so check if the locations don't conflict and rely on the train number for the rest
        if (!LocationUtil::isSameLocation(lhs.departureStation(), rhs.departureStation(), LocationUtil::CityLevel)
            || !LocationUtil::isSameLocation(lhs.arrivalStation(), rhs.arrivalStation(), LocationUtil::CityLevel)) {
            return false;
        }
    }

    // arrival times (when present) should either match exactly, or be almost the same at a matching arrival location
    // (tickets even for the same connection booked on the same day sometimes have slight variation in the arrival time...)
    if (conflictIfPresent(lhs.arrivalTime(), rhs.arrivalTime())) {
        qCDebug(CompareLog) << "arrival time" << lhs.arrivalTime() << rhs.arrivalTime();
        if (std::abs(lhs.arrivalTime().secsTo(rhs.arrivalTime())) > 180) {
            return false;
        }
        if (!LocationUtil::isSameLocation(lhs.arrivalStation(), rhs.arrivalStation(), LocationUtil::Exact)) {
            return false;
        }
    }

    // if we don't have train numbers, fall back to the less robust location comparison
    // we want at least one side to be an exact match to be reaonsbaly sure
    if (lhs.trainNumber().isEmpty() || rhs.trainNumber().isEmpty()) {
        qCDebug(CompareLog) << "missing train number" << lhs.trainNumber() << rhs.trainNumber();
        const auto depExact = LocationUtil::isSameLocation(lhs.departureStation(), rhs.departureStation(), LocationUtil::Exact);
        const auto arrExact = LocationUtil::isSameLocation(lhs.arrivalStation(), rhs.arrivalStation(), LocationUtil::Exact);
        if (depExact && arrExact) {
            return true;
        }
        if (!depExact && !arrExact) {
            return false;
        }
         return LocationUtil::isSameLocation(lhs.departureStation(), rhs.departureStation(), LocationUtil::CityLevel)
             || LocationUtil::isSameLocation(lhs.arrivalStation(), rhs.arrivalStation(), LocationUtil::CityLevel);

    }

    const auto isSameLine = isSameLineName(lhs.trainNumber(), rhs.trainNumber());
    qCDebug(CompareLog) << "left:" << lhs.trainName() << lhs.trainNumber() << lhs.departureTime();
    qCDebug(CompareLog) << "right:" << rhs.trainName() << rhs.trainNumber() << rhs.departureTime();
    qCDebug(CompareLog) << "same line:" << isSameLine;
    if (!conflictIfPresent(lhs.trainName(),rhs.trainName()) && isSameLine) {
        return true;
    }

    // handle dividing trains, ie. train number might differ
    if (isSameTrainType(lhs.trainNumber(), rhs.trainNumber()) && lhs.departureTime().isValid() && lhs.arrivalTime().isValid()) {
        const auto depExact = LocationUtil::isSameLocation(lhs.departureStation(), rhs.departureStation(), LocationUtil::Exact);
        const auto arrExact = LocationUtil::isSameLocation(lhs.arrivalStation(), rhs.arrivalStation(), LocationUtil::Exact);
        qCDebug(CompareLog) << "possible dividing train" << depExact << arrExact;
        return depExact && arrExact;
    }

    return false;
}

static bool isSameBusTrip(const BusTrip &lhs, const BusTrip &rhs)
{
    if (!equalAndPresent(lhs.departureTime(), rhs.departureTime())
      || conflictIfPresent(lhs.busNumber(), rhs.busNumber())
      || conflictIfPresent(lhs.arrivalTime(), rhs.arrivalTime())) {
        return false;
    }

    return equalAndPresent(lhs.busNumber(), rhs.busNumber()) ||
        (LocationUtil::isSameLocation(lhs.departureBusStop(), rhs.departureBusStop()) && LocationUtil::isSameLocation(lhs.arrivalBusStop(), rhs.arrivalBusStop()));
}

static bool isSameBoatTrip(const BoatTrip& lhs, const BoatTrip& rhs)
{
    return lhs.departureTime() == rhs.departureTime()
        && LocationUtil::isSameLocation(lhs.departureBoatTerminal(), rhs.departureBoatTerminal())
        && LocationUtil::isSameLocation(lhs.arrivalBoatTerminal(), rhs.arrivalBoatTerminal());
}

static bool isSameLocalBusiness(const LocalBusiness &lhs, const LocalBusiness &rhs)
{
    if (lhs.name().isEmpty() || rhs.name().isEmpty()) {
        return false;
    }

    if (!LocationUtil::isSameLocation(lhs, rhs)) {
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
static QString diffString(const QString &rawLhs, const QString &rawRhs)
{
    const auto lhs = StringUtil::normalize(rawLhs);
    const auto rhs = StringUtil::normalize(rawRhs);

    QString diff;
    // this is just a basic linear-time heuristic, this would need to be more something like
    // the Levenstein Distance algorithm
    for (int i = 0, j = 0; i < lhs.size() || j < rhs.size();) {
        if (i < lhs.size() && j < rhs.size() && lhs[i] == rhs[j]) {
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
    diff.remove(QLatin1StringView("MRS"));
    diff.remove(QLatin1StringView("MR"));
    diff.remove(QLatin1StringView("MS"));

    // if there's letters in the diff, we assume this is different
    for (const auto c : diff) {
        if (c.isLetter()) {
            return false;
        }
    }

    return true;
}

static bool isPartialName(const Person &fullName, const Person &partialName)
{
    if (fullName.familyName().isEmpty() || fullName.givenName().isEmpty() || !fullName.givenName().startsWith(partialName.givenName(), Qt::CaseInsensitive)) {
        return false;
    }
    return isNameEqualish(fullName.familyName(), partialName.name()) || isNameEqualish(fullName.familyName(), partialName.familyName());
}

bool MergeUtil::isSamePerson(const Person& lhs, const Person& rhs)
{
    if (isNameEqualish(lhs.name(), rhs.name()) ||
        (isNameEqualish(lhs.givenName(), rhs.givenName()) && isNameEqualish(lhs.familyName(), rhs.familyName()))) {
        return true;
    }
    if (isPartialName(lhs, rhs) || isPartialName(rhs, lhs)) {
        return true;
    }

    const auto lhsNameT = StringUtil::transliterate(lhs.name());
    const auto lhsGivenNameT = StringUtil::transliterate(lhs.givenName());
    const auto lhsFamilyNameT = StringUtil::transliterate(lhs.familyName());

    const auto rhsNameT = StringUtil::transliterate(rhs.name());
    const auto rhsGivenNameT = StringUtil::transliterate(rhs.givenName());
    const auto rhsFamilyNameT = StringUtil::transliterate(rhs.familyName());

    return isNameEqualish(lhsNameT, rhsNameT) ||
        (isNameEqualish(lhsGivenNameT, rhsGivenNameT) && isNameEqualish(lhsFamilyNameT, rhsFamilyNameT));
}

static bool isSameEvent(const Event &lhs, const Event &rhs)
{
    if (!equalAndPresent(lhs.startDate(), rhs.startDate())) {
        return false;
    }

    // event names can contain additional qualifiers, like for Adult/Child tickets,
    // those don't change the event though
    const auto namePrefix = StringUtil::prefixSimilarity(lhs.name(), rhs.name());
    return namePrefix == 1.0f || (namePrefix > 0.65f && LocationUtil::isSameLocation(lhs.location(), rhs.location(), LocationUtil::Exact));
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

bool MergeUtil::isSameIncidence(const QVariant &lhs, const QVariant &rhs)
{
    if (lhs.userType() != rhs.userType() || !JsonLd::canConvert<Reservation>(lhs) || !JsonLd::canConvert<Reservation>(rhs)) {
        return false;
    }

    // special case for LodgingReservation, their time range is in the Reservation object
    if (JsonLd::isA<LodgingReservation>(lhs)) {
        if (MergeUtil::isSame(lhs, rhs)) { // incremental updates can have deviating times, that is ok
            return true;
        }
        const auto lhsHotel = lhs.value<LodgingReservation>();
        const auto rhsHotel = rhs.value<LodgingReservation>();
        if (lhsHotel.checkinTime().date() != rhsHotel.checkinTime().date() ||
            lhsHotel.checkoutTime().date() != rhsHotel.checkoutTime().date()) {
            return false;
        }
    }

    const auto lhsTrip = JsonLd::convert<Reservation>(lhs).reservationFor();
    const auto rhsTrip = JsonLd::convert<Reservation>(rhs).reservationFor();
    return MergeUtil::isSame(lhsTrip, rhsTrip);
}

static Airline mergeValue(const Airline &lhs, const Airline &rhs)
{
    auto a = JsonLdDocument::apply(lhs, rhs).value<Airline>();
    a.setName(StringUtil::betterString(lhs.name(), rhs.name()).toString());
    return a;
}

static QDateTime mergeValue(const QDateTime &lhs, const QDateTime &rhs)
{
    // if both sides have a timezone, prefer non-UTC
    if (lhs.isValid() && lhs.timeSpec() == Qt::TimeZone && rhs.isValid() && rhs.timeSpec() == Qt::TimeZone) {
        return rhs.timeZone() == QTimeZone::utc() ? lhs : rhs;
    }
    // prefer value with timezone
    return lhs.isValid() && lhs.timeSpec() == Qt::TimeZone && rhs.timeSpec() != Qt::TimeZone ? lhs : rhs;
}

static Person mergeValue(const Person &lhs, const Person &rhs)
{
    auto p = JsonLdDocument::apply(lhs, rhs).value<Person>();
    p.setFamilyName(StringUtil::betterString(lhs.familyName(), rhs.familyName()).toString());
    p.setGivenName(StringUtil::betterString(lhs.givenName(), rhs.givenName()).toString());
    p.setName(StringUtil::betterString(lhs.name(), rhs.name()).toString());
    return p;
}

static QVariantList mergeSubjectOf(const QVariantList &lhs, const QVariantList &rhs)
{
    std::set<QString> mergedSet;
    for (const auto &v : lhs) {
        if (v.userType() != QMetaType::QString) {
            return rhs.isEmpty() ? lhs : rhs;
        }
        mergedSet.insert(v.toString());
    }
    for (const auto &v : rhs) {
        if (v.userType() != QMetaType::QString) {
            return rhs.isEmpty() ? lhs : rhs;
        }
        mergedSet.insert(v.toString());
    }

    QVariantList result;
    result.reserve(mergedSet.size());
    std::copy(mergedSet.begin(), mergedSet.end(), std::back_inserter(result));
    return result;
}

static int ticketTokenSize(const QVariant &v)
{
    if (v.userType() == QMetaType::QString) {
        return v.toString().size();
    }
    if (v.userType() == QMetaType::QByteArray) {
        return v.toByteArray().size();
    }
    return 0;
}

static Ticket mergeValue(const Ticket &lhs, const Ticket &rhs)
{
    auto t = JsonLdDocument::apply(lhs, rhs).value<Ticket>();
    // prefer barcode ticket tokens over URLs
    if (t.ticketTokenType() == Token::Url && lhs.ticketTokenType() != Token::Url && lhs.ticketTokenType() != Token::Unknown) {
        t.setTicketToken(lhs.ticketToken());
    } else if (lhs.ticketTokenType() != Token::Url && rhs.ticketTokenType() != Token::Url
            && lhs.ticketTokenType() != Token::Unknown && rhs.ticketTokenType() != Token::Unknown) {
        t.setTicketToken(ticketTokenSize(lhs.ticketTokenData()) > ticketTokenSize(rhs.ticketTokenData()) ? lhs.ticketToken() : rhs.ticketToken());
    }
    return t;
}

QVariant MergeUtil::merge(const QVariant &lhs, const QVariant &rhs)
{
    if (rhs.isNull()) {
        return lhs;
    }
    if (lhs.isNull()) {
        return rhs;
    }
    if (lhs.typeId() != rhs.typeId() && !isSubType(lhs, rhs) && !isSubType(rhs, lhs)) {
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

    // handle simple value types
    if (lhs.typeId() == rhs.typeId()) {
        const auto mt = lhs.typeId();
        if (mt == qMetaTypeId<Airline>()) {
            return mergeValue(lhs.value<Airline>(), rhs.value<Airline>());
        }
        if (mt == qMetaTypeId<Person>()) {
            return mergeValue(lhs.value<Person>(), rhs.value<Person>());
        }
        if (mt == QMetaType::QDateTime) {
            return mergeValue(lhs.toDateTime(), rhs.toDateTime());
        }
        if (mt == qMetaTypeId<Ticket>()) {
            return mergeValue(lhs.value<Ticket>(), rhs.value<Ticket>());
        }
        if (mt == QMetaType::QString) {
            return StringUtil::betterString(lhs.toString(), rhs.toString()).toString();
        }
        if ((QMetaType(mt).flags() & QMetaType::IsGadget) == 0 && !JsonLd::valueIsNull(rhs)) {
            return rhs;
        }
    }

    auto res = lhs;
    if (lhs.userType() != rhs.userType() && isSubType(rhs, lhs)) {
        res = rhs;
    }

    const auto mt = QMetaType(res.typeId());
    const auto mo = QMetaType(res.typeId()).metaObject();
    if (mo && (mt.flags() & QMetaType::IsGadget)) {
        for (int i = 0; i < mo->propertyCount(); ++i) {
            const auto prop = mo->property(i);
            if (!prop.isStored()) {
                continue;
            }

            const auto lv = prop.readOnGadget(lhs.constData());
            auto rv = prop.readOnGadget(rhs.constData());
            if (rv.typeId() == QMetaType::QVariantList && std::strcmp(prop.name(), "subjectOf") == 0) {
                rv = mergeSubjectOf(lv.toList(), rv.toList());
            } else {
                rv = MergeUtil::merge(lv, rv);
            }

            if (!JsonLd::valueIsNull(rv)) {
                prop.writeOnGadget(res.data(), rv);
            }
        }
    }

    return res;
}

bool isMinimalCancelationFor(const Reservation &res, const Reservation &cancel)
{
    if (cancel.reservationStatus() != Reservation::ReservationCancelled) {
        return false;
    }
    if (!equalAndPresent(res.reservationNumber(), cancel.reservationNumber())) {
        return false;
    }
    if (!cancel.modifiedTime().isValid() || !cancel.reservationFor().isNull()) {
        return false;
    }
    return SortUtil::startDateTime(res) > cancel.modifiedTime();
}

static bool isCompatibleLocationChange(const QVariant &lhs, const QVariant &rhs)
{
    const bool lhsTrainOrBus = JsonLd::isA<TrainReservation>(lhs) || JsonLd::isA<BusReservation>(lhs);
    const bool rhsTrainOrBus = JsonLd::isA<TrainReservation>(rhs) || JsonLd::isA<BusReservation>(rhs);
    return (lhsTrainOrBus && rhsTrainOrBus) || (JsonLd::isA<FlightReservation>(lhs) && JsonLd::isA<FlightReservation>(rhs));
}

bool MergeUtil::hasSameDeparture(const QVariant &lhs, const QVariant &rhs)
{
    if (!isCompatibleLocationChange(lhs, rhs)) {
        return false;
    }
    const auto lhsRes = JsonLd::convert<Reservation>(lhs);
    const auto rhsRes = JsonLd::convert<Reservation>(rhs);
    if (!isSameReservation(lhsRes, rhsRes)) {
        return false;
    }

    if (SortUtil::hasStartTime(lhs) && SortUtil::hasStartTime(rhs)) {
        if (SortUtil::startDateTime(lhs) != SortUtil::startDateTime(rhs)) {
            return false;
        }
        return LocationUtil::isSameLocation(LocationUtil::departureLocation(lhs), LocationUtil::departureLocation(rhs), LocationUtil::Exact);
    }
    if (SortUtil::hasStartTime(lhs) || SortUtil::hasStartTime(rhs)) {
        if (SortUtil::startDateTime(lhs).date() != SortUtil::startDateTime(rhs).date()) {
            return false;
        }
        return LocationUtil::isSameLocation(LocationUtil::departureLocation(lhs), LocationUtil::departureLocation(rhs), LocationUtil::CityLevel);
    }

    return false;
}

bool MergeUtil::hasSameArrival(const QVariant &lhs, const QVariant &rhs)
{
    if (!isCompatibleLocationChange(lhs, rhs)) {
        return false;
    }
    const auto lhsRes = JsonLd::convert<Reservation>(lhs);
    const auto rhsRes = JsonLd::convert<Reservation>(rhs);
    if (!isSameReservation(lhsRes, rhsRes)) {
        return false;
    }

    if (SortUtil::hasEndTime(lhs) && SortUtil::hasEndTime(rhs)) {
        if (SortUtil::endDateTime(lhs) != SortUtil::endDateTime(rhs)) {
            return false;
        }
        return LocationUtil::isSameLocation(LocationUtil::arrivalLocation(lhs), LocationUtil::arrivalLocation(rhs), LocationUtil::Exact);
    }

    if (SortUtil::hasEndTime(lhs) || SortUtil::hasEndTime(rhs)) {
        if (SortUtil::endDateTime(lhs).date() != SortUtil::endDateTime(rhs).date()) {
            return false;
        }
        return LocationUtil::isSameLocation(LocationUtil::arrivalLocation(lhs), LocationUtil::arrivalLocation(rhs), LocationUtil::CityLevel);
    }

    return false;
}

void MergeUtil::registerComparator(int metaTypeId, std::function<bool (const QVariant&, const QVariant&)> &&func)
{
    auto it = std::lower_bound(s_mergeCompareFuncs.begin(), s_mergeCompareFuncs.end(), metaTypeId);
    if (it != s_mergeCompareFuncs.end() && (*it).metaTypeId == metaTypeId) {
        (*it).func = std::move(func);
    } else {
        s_mergeCompareFuncs.insert(it, {metaTypeId, std::move(func)});
    }
}
