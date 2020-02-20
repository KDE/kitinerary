/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

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

#include "extractorvalidator_p.h"
#include "validator-logging.h"

#include <KItinerary/BusTrip>
#include <KItinerary/Event>
#include <KItinerary/Flight>
#include <KItinerary/Place>
#include <KItinerary/RentalCar>
#include <KItinerary/Reservation>
#include <KItinerary/Taxi>
#include <KItinerary/TrainTrip>
#include <KItinerary/Visit>

#include <QDateTime>

using namespace KItinerary;

static bool filterLodgingReservation(const LodgingReservation &res)
{
    return res.checkinTime().isValid() && res.checkoutTime().isValid() && res.checkinTime() <= res.checkoutTime();
}

static bool filterAirport(const Airport &airport)
{
    return !airport.iataCode().isEmpty() || !airport.name().isEmpty();
}

static bool filterFlight(const Flight &flight)
{
    // this will be valid if either boarding time, departure time or departure day is set
    const auto validDate = flight.departureDay().isValid();
    return filterAirport(flight.departureAirport())
           && filterAirport(flight.arrivalAirport())
           && validDate;
}

template <typename T>
static bool filterTrainOrBusStation(const T &station)
{
    return !station.name().isEmpty();
}

static bool filterTrainTrip(const TrainTrip &trip)
{
    return filterTrainOrBusStation(trip.departureStation())
           && filterTrainOrBusStation(trip.arrivalStation())
           && trip.departureDay().isValid();
}

static bool filterBusTrip(const BusTrip &trip)
{
    return filterTrainOrBusStation(trip.departureBusStop())
           && filterTrainOrBusStation(trip.arrivalBusStop())
           && trip.departureTime().isValid() && trip.arrivalTime().isValid();
}

static bool filterEvent(const Event &event)
{
    return !event.name().isEmpty() && event.startDate().isValid();
}

static bool filterFoodReservation(const FoodEstablishmentReservation &res)
{
    return res.startTime().isValid();
}

static bool filterFoodEstablishment(const FoodEstablishment &res)
{
    return !res.name().isEmpty();
}

static bool filterLodgingBusiness(const LodgingBusiness &hotel)
{
    return !hotel.name().isEmpty();
}

bool ExtractorValidator::isValidElement(const QVariant &elem)
{
    // reservation types
    if (JsonLd::canConvert<Reservation>(elem)) {
        const auto res = JsonLd::convert<Reservation>(elem);
        if (!isValidElement(res.reservationFor())) {
            qCDebug(ValidatorLog) << "Reservation element discarded due to rejected reservationFor property:" << elem.typeName();
            return false;
        }

        if (JsonLd::isA<LodgingReservation>(elem)) {
            return filterLodgingReservation(elem.value<LodgingReservation>());
        }
        if (JsonLd::isA<FoodEstablishmentReservation>(elem)) {
            return filterFoodReservation(elem.value<FoodEstablishmentReservation>());
        }

        return true;
    }

    // reservationFor types
    if (JsonLd::isA<Flight>(elem)) {
        return filterFlight(elem.value<Flight>());
    }
    if (JsonLd::isA<TrainTrip>(elem)) {
        return filterTrainTrip(elem.value<TrainTrip>());
    }
    if (JsonLd::isA<BusTrip>(elem)) {
        return filterBusTrip(elem.value<BusTrip>());
    }
    if (JsonLd::isA<Event>(elem)) {
        return filterEvent(elem.value<Event>());
    }
    if (JsonLd::isA<FoodEstablishment>(elem)) {
        return filterFoodEstablishment(elem.value<FoodEstablishment>());
    }
    if (JsonLd::isA<LodgingBusiness>(elem)) {
        return filterLodgingBusiness(elem.value<LodgingBusiness>());
    }

    // types without specific filters yet
    if (JsonLd::isA<TouristAttractionVisit>(elem) ||
        JsonLd::isA<RentalCar>(elem) ||
        JsonLd::isA<Taxi>(elem)
    ) {
        return true;
    }

    // unknown top-level type
    qCDebug(ValidatorLog) << "Element discarded due to unsupported top-level type:" << elem.typeName();
    return false;
}
