/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "sortutil.h"

#include <KItinerary/BusTrip>
#include <KItinerary/Event>
#include <KItinerary/Flight>
#include <KItinerary/MergeUtil>
#include <KItinerary/Person>
#include <KItinerary/Place>
#include <KItinerary/RentalCar>
#include <KItinerary/Reservation>
#include <KItinerary/Ticket>
#include <KItinerary/TrainTrip>
#include <KItinerary/Visit>

#include <QDateTime>
#include <QTimeZone>

using namespace KItinerary;

QDateTime SortUtil::startDateTime(const QVariant &elem)
{
    // reservation types
    if (JsonLd::isA<FoodEstablishmentReservation>(elem)) {
        return elem.value<FoodEstablishmentReservation>().startTime();
    }
    if (JsonLd::isA<LodgingReservation>(elem)) {
        const auto hotel = elem.value<LodgingReservation>();
        // hotel checkin/checkout is always considered the first/last thing of the day
        auto dt = QDateTime(hotel.checkinTime().date(), QTime(23, 59, 59));
        if (hotel.checkinTime().timeSpec() == Qt::TimeZone) {
            dt.setTimeZone(hotel.checkinTime().timeZone());
        }
        return dt;
    }
    if (JsonLd::isA<RentalCarReservation>(elem)) {
        return elem.value<RentalCarReservation>().pickupTime();
    }
    if (JsonLd::isA<TaxiReservation>(elem)) {
        return elem.value<TaxiReservation>().pickupTime();
    }
    if (JsonLd::canConvert<Reservation>(elem)) {
        const auto res = JsonLd::convert<Reservation>(elem).reservationFor();
        return startDateTime(res);
    }
    if (JsonLd::isA<TouristAttractionVisit>(elem)) {
        return elem.value<TouristAttractionVisit>().arrivalTime();
    }

    // "reservationFor" types
    if (JsonLd::isA<Flight>(elem)) {
        const auto flight = elem.value<Flight>();
        if (flight.departureTime().isValid()) {
            return flight.departureTime();
        }
        if (flight.boardingTime().isValid()) {
            return flight.boardingTime();
        }
        return QDateTime(flight.departureDay(), QTime(23, 59, 59));
    }
    if (JsonLd::isA<TrainTrip>(elem)) {
        const auto trip = elem.value<TrainTrip>();
        if (trip.departureTime().isValid()) {
            return trip.departureTime();
        }
        return QDateTime(trip.departureDay(), QTime(23, 59, 59));
    }
    if (JsonLd::isA<BusTrip>(elem)) {
        return elem.value<BusTrip>().departureTime();
    }
    if (JsonLd::isA<Event>(elem)) {
        return elem.value<Event>().startDate();
    }

    return {};
}

QDateTime SortUtil::endDateTime(const QVariant &elem)
{
    // reservation types
    if (JsonLd::isA<FoodEstablishmentReservation>(elem)) {
        auto endTime = elem.value<FoodEstablishmentReservation>().endTime();
        if (!endTime.isValid()) {
            endTime = QDateTime(elem.value<FoodEstablishmentReservation>().startTime().date(), QTime(23, 59, 59));
        }
        return endTime;
    }
    if (JsonLd::isA<RentalCarReservation>(elem)) {
        return elem.value<RentalCarReservation>().dropoffTime();
    }
    if (JsonLd::isA<LodgingReservation>(elem)) {
        const auto hotel = elem.value<LodgingReservation>();
        // hotel checkin/checkout is always considered the first/last thing of the day
        auto dt = QDateTime(hotel.checkoutTime().date(), QTime(0, 0, 0));
        if (hotel.checkoutTime().timeSpec() == Qt::TimeZone) {
            dt.setTimeZone(hotel.checkoutTime().timeZone());
        }
        return dt;
    }
    if (JsonLd::canConvert<Reservation>(elem)) {
        const auto res = JsonLd::convert<Reservation>(elem).reservationFor();
        return endDateTime(res);
    }
    if (JsonLd::isA<TouristAttractionVisit>(elem)) {
        return elem.value<TouristAttractionVisit>().departureTime();
    }

    // "reservationFor" types
    if (JsonLd::isA<Event>(elem)) {
        return elem.value<Event>().endDate();
    }
    if (JsonLd::isA<Flight>(elem)) {
        const auto flight = elem.value<Flight>();
        if (flight.arrivalTime().isValid()) {
            return flight.arrivalTime();
        }
        return QDateTime(flight.departureDay(), QTime(23, 59, 59));
    }
    if (JsonLd::isA<TrainTrip>(elem)) {
        const auto trip = elem.value<TrainTrip>();
        if (trip.arrivalTime().isValid()) {
            return trip.arrivalTime();
        }
        return QDateTime(trip.departureDay(), QTime(23, 59, 59));
    }
    if (JsonLd::isA<BusTrip>(elem)) {
        return elem.value<BusTrip>().arrivalTime();
    }

    return {};
}

bool SortUtil::isBefore(const QVariant &lhs, const QVariant &rhs)
{
    if (startDateTime(lhs) == startDateTime(rhs) && lhs.userType() == rhs.userType() && JsonLd::canConvert<Reservation>(lhs)) {
        // for multi-traveler reservations, sort by traveler name to achieve a stable result
        const auto lhsRes = JsonLd::convert<Reservation>(lhs);
        const auto rhsRes = JsonLd::convert<Reservation>(rhs);
        if (!lhsRes.underName().isNull() && !rhsRes.underName().isNull() && MergeUtil::isSame(lhsRes.reservationFor(), rhsRes.reservationFor())) {
            const auto lhsUN = lhsRes.underName().value<Person>();
            const auto rhsUN = rhsRes.underName().value<Person>();

            // for multi-ticket reservations for the same person, sort by ticket name
            if (lhsUN.name() == rhsUN.name()) {
                const auto lhsTicket = lhsRes.reservedTicket().value<Ticket>();
                const auto rhsTicket = rhsRes.reservedTicket().value<Ticket>();
                return lhsTicket.name() < rhsTicket.name();
            }
            return lhsUN.name() < rhsUN.name();
        }
    }
    return startDateTime(lhs) < startDateTime(rhs);
}