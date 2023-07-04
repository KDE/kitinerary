/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "extractorvalidator.h"
#include "validator-logging.h"

#include <KItinerary/BoatTrip>
#include <KItinerary/BusTrip>
#include <KItinerary/Event>
#include <KItinerary/Flight>
#include <KItinerary/Place>
#include <KItinerary/ProgramMembership>
#include <KItinerary/RentalCar>
#include <KItinerary/Reservation>
#include <KItinerary/Taxi>
#include <KItinerary/Ticket>
#include <KItinerary/TrainTrip>
#include <KItinerary/Visit>

#include <QDateTime>

using namespace KItinerary;

namespace KItinerary {
class ExtractorValidatorPrivate {
public:
    bool isSupportedTopLevelType(const QVariant &elem) const;
    bool filterElement(const QVariant &elem) const;

    bool filterLodgingReservation(const LodgingReservation &res) const;
    bool filterAirport(const Airport &airport) const;
    bool filterFlight(const Flight &flight) const;
    bool filterTrainTrip(const TrainTrip &trip) const;
    bool filterBusTrip(const BusTrip &trip) const;
    bool filterBoatTrip(const BoatTrip &trip) const;
    bool filterEvent(const Event &event) const;
    bool filterFoodReservation(const FoodEstablishmentReservation &res) const;
    bool filterLocalBusiness(const LocalBusiness &business) const;
    bool filterReservation(const Reservation &res) const;
    bool filterProgramMembership(const ProgramMembership &program) const;
    bool filterTicket(const Ticket &ticket) const;

    std::vector<const QMetaObject*> m_acceptedTypes;
    bool m_onlyComplete = true;
};
}

ExtractorValidator::ExtractorValidator() :
    d(new ExtractorValidatorPrivate)
{}

ExtractorValidator::~ExtractorValidator() = default;

void ExtractorValidator::setAcceptedTypes(std::vector<const QMetaObject*> &&accptedTypes)
{
    d->m_acceptedTypes = std::move(accptedTypes);
}

void ExtractorValidator::setAcceptOnlyCompleteElements(bool completeOnly)
{
    d->m_onlyComplete = completeOnly;
}


bool ExtractorValidatorPrivate::filterLodgingReservation(const LodgingReservation &res) const
{
    return res.checkinTime().isValid() && res.checkoutTime().isValid() && res.checkinTime() <= res.checkoutTime();
}

bool ExtractorValidatorPrivate::filterAirport(const Airport &airport) const
{
    return !airport.iataCode().isEmpty() || !airport.name().isEmpty();
}

bool ExtractorValidatorPrivate::filterFlight(const Flight &flight) const
{
    // this will be valid if either boarding time, departure time or departure day is set
    const auto validDate = flight.departureDay().isValid();
    return filterAirport(flight.departureAirport())
           && filterAirport(flight.arrivalAirport())
           && validDate;
}

template <typename T>
static bool filterPlace(const T &place)
{
    return !place.name().isEmpty();
}

bool ExtractorValidatorPrivate::filterTrainTrip(const TrainTrip &trip) const
{
    return filterPlace(trip.departureStation())
           && filterPlace(trip.arrivalStation())
           && trip.departureDay().isValid();
}

bool ExtractorValidatorPrivate::filterBusTrip(const BusTrip &trip) const
{
    return filterPlace(trip.departureBusStop())
           && filterPlace(trip.arrivalBusStop())
           && trip.departureTime().isValid();
}

bool ExtractorValidatorPrivate::filterBoatTrip(const BoatTrip &trip) const
{
    return filterPlace(trip.departureBoatTerminal())
        && filterPlace(trip.arrivalBoatTerminal())
        && trip.departureTime().isValid()
        && trip.arrivalTime().isValid();
}

bool ExtractorValidatorPrivate::filterEvent(const Event &event) const
{
    return !event.name().isEmpty() && event.startDate().isValid();
}

bool ExtractorValidatorPrivate::filterFoodReservation(const FoodEstablishmentReservation &res) const
{
    return res.startTime().isValid();
}

bool ExtractorValidatorPrivate::filterLocalBusiness(const LocalBusiness &business) const
{
    return !business.name().isEmpty();
}

bool ExtractorValidatorPrivate::filterReservation(const Reservation &res) const
{
    if (!m_onlyComplete) { // accept minimal cancellation elements
        if (res.reservationFor().isNull()
            && res.modifiedTime().isValid()
            && !res.reservationNumber().isEmpty()
            && res.reservationStatus() == Reservation::ReservationCancelled)
        {
            return true;
        }
    }

    if (!filterElement(res.reservationFor())) {
        qCDebug(ValidatorLog) << "Reservation element discarded due to rejected reservationFor property:" << res.reservationFor().typeName();
        return false;
    }
    return true;
}

bool ExtractorValidatorPrivate::filterProgramMembership(const ProgramMembership &program) const
{
    return (!program.membershipNumber().isEmpty() || !program.token().isEmpty()) && !program.programName().isEmpty();
}

bool ExtractorValidatorPrivate::filterTicket(const Ticket &ticket) const
{
    return !ticket.ticketToken().isEmpty() && !ticket.name().isEmpty();
}

template <typename T, bool (ExtractorValidatorPrivate::*F)(const T&) const>
static inline bool callFilterWithType(const ExtractorValidatorPrivate *d, const QVariant &v)
{
    // JsonLd::canConvert<T>(v) is guaranteed by walking up the meta object tree here
    return (d->*F)(JsonLd::convert<T>(v));
}

#define FILTER(Type, Func) { &Type::staticMetaObject, callFilterWithType<Type, &ExtractorValidatorPrivate::Func> }
struct {
    const QMetaObject *metaObject;
    bool (*filter)(const ExtractorValidatorPrivate *d, const QVariant &v);
} static const filter_func_map[] {
    FILTER(Flight, filterFlight),
    FILTER(TrainTrip, filterTrainTrip),
    FILTER(BusTrip, filterBusTrip),
    FILTER(BoatTrip, filterBoatTrip),
    FILTER(KItinerary::Event, filterEvent),
    FILTER(LocalBusiness, filterLocalBusiness),
    FILTER(FoodEstablishmentReservation, filterFoodReservation),
    FILTER(LodgingReservation, filterLodgingReservation),
    FILTER(Reservation, filterReservation),
    FILTER(ProgramMembership, filterProgramMembership),
    FILTER(Ticket, filterTicket),
};
#undef FILTER

bool ExtractorValidatorPrivate::filterElement(const QVariant &elem) const
{
    auto mo = QMetaType(elem.userType()).metaObject();
    if (!mo) {
        qCDebug(ValidatorLog) << "Element discarded due to non-gadget type:" << elem.typeName();
        return false;
    }
    while (mo) {
        for (const auto &f : filter_func_map) {
            if (f.metaObject != mo) {
                continue;
            }
            if (!f.filter(this, elem)) {
                return false;
            }
            break;
        }
        mo = mo->superClass();
    }
    return true;
}

bool ExtractorValidatorPrivate::isSupportedTopLevelType(const QVariant &elem) const
{
    if (m_acceptedTypes.empty()) { // nothing configured, we accept everything
        return true;
    }

    auto mo = QMetaType(elem.userType()).metaObject();
    if (!mo) {
        qCDebug(ValidatorLog) << "Element discarded due to non-gadget top-level type:" << elem.typeName();
        return false;
    }
    while (mo) {
        const auto it = std::find(m_acceptedTypes.begin(), m_acceptedTypes.end(), mo);
        if (it != m_acceptedTypes.end()) {
            return true;
        }
        mo = mo->superClass();
    }
    return false;
}

bool ExtractorValidator::isValidElement(const QVariant &elem) const
{
    // check this is an allowed top-level type
    if (!d->isSupportedTopLevelType(elem)) {
        qCDebug(ValidatorLog) << "Element discarded due to unsupported top-level type:" << elem.typeName();
        return false;
    }

    // apply type-specific filter functions
    return d->filterElement(elem);
}
