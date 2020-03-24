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

#include "extractorvalidator.h"
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

static bool filterElement(const QVariant &elem);

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

static bool filterLocalBusiness(const LocalBusiness &business)
{
    return !business.name().isEmpty();
}

static bool filterReservation(const Reservation &res)
{
    if (!filterElement(res.reservationFor())) {
        qCDebug(ValidatorLog) << "Reservation element discarded due to rejected reservationFor property:" << res.reservationFor().typeName();
        return false;
    }
    return true;
}

template <typename T, bool (*F)(const T&)>
static inline bool callFilterWithType(const QVariant &v)
{
    // JsonLd::canConvert<T>(v) is guaranteed by walking up the meta object tree here
    return F(JsonLd::convert<T>(v));
}

#define FILTER(Type, Func) { &Type::staticMetaObject, callFilterWithType<Type, Func> }
struct {
    const QMetaObject *metaObject;
    bool (*filter)(const QVariant &v);
} static const filter_func_map[] {
    FILTER(Flight, filterFlight),
    FILTER(TrainTrip, filterTrainTrip),
    FILTER(BusTrip, filterBusTrip),
    FILTER(Event, filterEvent),
    FILTER(LocalBusiness, filterLocalBusiness),
    FILTER(FoodEstablishmentReservation, filterFoodReservation),
    FILTER(LodgingReservation, filterLodgingReservation),
    FILTER(Reservation, filterReservation),
};
#undef FILTER

static bool filterElement(const QVariant &elem)
{
    auto mo = QMetaType::metaObjectForType(elem.userType());
    if (!mo) {
        qCDebug(ValidatorLog) << "Element discared due to non-gadget type:" << elem.typeName();
        return false;
    }
    while (mo) {
        for (const auto &f : filter_func_map) {
            if (f.metaObject != mo) {
                continue;
            }
            if (!f.filter(elem)) {
                return false;
            }
            break;
        }
        mo = mo->superClass();
    }
    return true;
}

// TODO this default is merely to retain behavior compat for now
// eventually this should be configured by the user to  the subset they need
static const QMetaObject* supported_type_table[] = {
    // reservation types
    &FlightReservation::staticMetaObject,
    &TrainReservation::staticMetaObject,
    &BusReservation::staticMetaObject,
    &RentalCarReservation::staticMetaObject,
    &TaxiReservation::staticMetaObject,
    &EventReservation::staticMetaObject,
    &FoodEstablishmentReservation::staticMetaObject,
    &LodgingReservation::staticMetaObject,

    // reservationFor types
    &Flight::staticMetaObject,
    &TrainTrip::staticMetaObject,
    &BusTrip::staticMetaObject,
    &RentalCar::staticMetaObject,
    &Taxi::staticMetaObject,
    &Event::staticMetaObject,
    &TouristAttractionVisit::staticMetaObject,
    &FoodEstablishment::staticMetaObject,

    // PBI types
    &LocalBusiness::staticMetaObject,
};

static bool isSupportedTopLevelType(const QVariant &elem)
{
    auto mo = QMetaType::metaObjectForType(elem.userType());
    if (!mo) {
        qCDebug(ValidatorLog) << "Element discared due to non-gadget top-level type:" << elem.typeName();
        return false;
    }
    while (mo) {
        for (const auto &t : supported_type_table) {
            if (t == mo) {
                return true;
            }
        }
        mo = mo->superClass();
    }
    return false;
}

bool ExtractorValidator::isValidElement(const QVariant &elem)
{
    // check this is an allowed top-level type
    if (!isSupportedTopLevelType(elem)) {
        qCDebug(ValidatorLog) << "Element discarded due to unsupported top-level type:" << elem.typeName();
        return false;
    }

    // apply type-specific filter functions
    return filterElement(elem);
}
