/*
   Copyright (c) 2017 Volker Krause <vkrause@kde.org>

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   This library is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
   License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.
*/

#include "config-kitinerary.h"
#include "extractorpostprocessor.h"

#include "iatabcbpparser.h"
#include "jsonlddocument.h"
#include "logging.h"
#include "mergeutil.h"
#include "sortutil.h"

#include <KItinerary/Action>
#include <KItinerary/AirportDb>
#include <KItinerary/BusTrip>
#include <KItinerary/Event>
#include <KItinerary/Flight>
#include <KItinerary/Organization>
#include <KItinerary/Person>
#include <KItinerary/Place>
#include <KItinerary/Reservation>
#include <KItinerary/Ticket>
#include <KItinerary/TrainTrip>
#include <KItinerary/TrainStationDb>
#include <KItinerary/Visit>

#ifdef HAVE_KCONTACTS
#include <KContacts/Address>
#endif

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTimeZone>
#include <QUrl>

#include <algorithm>

using namespace KItinerary;
namespace KItinerary {
class ExtractorPostprocessorPrivate
{
public:
    void mergeOrAppend(const QVariant &elem);

    QVariant processFlightReservation(FlightReservation res) const;
    Flight processFlight(Flight flight) const;
    Airport processAirport(Airport airport) const;
    Airline processAirline(Airline airline) const;
    QDateTime processFlightTime(QDateTime dt, const Flight &flight, const Airport &airport) const;

    TrainReservation processTrainReservation(TrainReservation res) const;
    TrainTrip processTrainTrip(TrainTrip trip) const;
    TrainStation processTrainStation(TrainStation station) const;
    QDateTime processTrainTripTime(QDateTime dt, const TrainStation &station) const;

    BusReservation processBusReservation(BusReservation res) const;
    BusTrip processBusTrip(BusTrip trip) const;

    LodgingReservation processLodgingReservation(LodgingReservation res) const;
    FoodEstablishmentReservation processFoodEstablishmentReservation(FoodEstablishmentReservation res) const;
    TouristAttractionVisit processTouristAttractionVisit(TouristAttractionVisit visit) const;
    EventReservation processEventReservation(EventReservation res) const;
    RentalCarReservation processRentalCarReservation(RentalCarReservation res) const;
    Event processEvent(Event event) const;

    template <typename T> T processReservation(T res) const;
    Person processPerson(Person person) const;
    template <typename T> T processPlace(T place) const;
    QVariantList processActions(QVariantList actions) const;

    bool filterReservation(const QVariant &res) const;
    bool filterLodgingReservation(const LodgingReservation &res) const;
    bool filterFlight(const Flight &flight) const;
    bool filterAirport(const Airport &airport) const;
    template <typename T> bool filterTrainOrBusTrip(const T &trip) const;
    template <typename T> bool filterTrainOrBusStation(const T &station) const;
    bool filterEventReservation(const EventReservation &res) const;

    QVector<QVariant> m_data;
    QDateTime m_contextDate;
    bool m_resultFinalized = false;
};
}

ExtractorPostprocessor::ExtractorPostprocessor()
    : d(new ExtractorPostprocessorPrivate)
{
}

ExtractorPostprocessor::ExtractorPostprocessor(ExtractorPostprocessor &&) noexcept = default;
ExtractorPostprocessor::~ExtractorPostprocessor() = default;

void ExtractorPostprocessor::process(const QVector<QVariant> &data)
{
    d->m_resultFinalized = false;
    d->m_data.reserve(d->m_data.size() + data.size());
    for (auto elem : data) {
        if (JsonLd::isA<FlightReservation>(elem)) {
            elem = d->processFlightReservation(elem.value<FlightReservation>());
        } else if (JsonLd::isA<TrainReservation>(elem)) {
            elem = d->processTrainReservation(elem.value<TrainReservation>());
        } else if (JsonLd::isA<LodgingReservation>(elem)) {
            elem = d->processLodgingReservation(elem.value<LodgingReservation>());
        } else if (JsonLd::isA<FoodEstablishmentReservation>(elem)) {
            elem = d->processFoodEstablishmentReservation(elem.value<FoodEstablishmentReservation>());
        } else if (JsonLd::isA<TouristAttractionVisit>(elem)) {
            elem = d->processTouristAttractionVisit(elem.value<TouristAttractionVisit>());
        } else if (JsonLd::isA<BusReservation>(elem)) {
            elem = d->processBusReservation(elem.value<BusReservation>());
        } else if (JsonLd::isA<EventReservation>(elem)) {
            elem = d->processEventReservation(elem.value<EventReservation>());
        } else if (JsonLd::isA<RentalCarReservation>(elem)) {
            elem = d->processRentalCarReservation(elem.value<RentalCarReservation>());
        }

        d->mergeOrAppend(elem);
    }
}

QVector<QVariant> ExtractorPostprocessor::result() const
{
    if (!d->m_resultFinalized) {
        for (auto it = d->m_data.begin(); it != d->m_data.end();) {
            if (d->filterReservation(*it)) {
                ++it;
            } else {
                //qCDebug(Log).noquote() << "Discarding element:" << QJsonDocument(JsonLdDocument::toJson({*it})).toJson();
                it = d->m_data.erase(it);
            }
        }
        d->m_resultFinalized = true;
    }

    std::stable_sort(d->m_data.begin(), d->m_data.end(), SortUtil::isBefore);
    return d->m_data;
}

void ExtractorPostprocessor::setContextDate(const QDateTime& dt)
{
    d->m_contextDate = dt;
}

void ExtractorPostprocessorPrivate::mergeOrAppend(const QVariant &elem)
{
    const auto it = std::find_if(m_data.begin(), m_data.end(), [elem](const QVariant &other) {
        return MergeUtil::isSame(elem, other);
    });

    if (it == m_data.end()) {
        m_data.push_back(elem);
    } else {
        *it = JsonLdDocument::apply(*it, elem);
    }
}

QVariant ExtractorPostprocessorPrivate::processFlightReservation(FlightReservation res) const
{
    // expand ticketToken for IATA BCBP data
    const auto bcbp = res.reservedTicket().value<Ticket>().ticketTokenData();
    if (!bcbp.isEmpty()) {
        const auto bcbpData = IataBcbpParser::parse(bcbp, m_contextDate.date());
        if (bcbpData.size() == 1) {
            res = JsonLdDocument::apply(bcbpData.at(0), res).value<FlightReservation>();
        } else {
            for (const auto &data : bcbpData) {
                if (MergeUtil::isSame(res, data)) {
                    res = JsonLdDocument::apply(data, res).value<FlightReservation>();
                    break;
                }
            }
        }
    }

    res.setReservationFor(processFlight(res.reservationFor().value<Flight>()));
    return processReservation(res);
}

Flight ExtractorPostprocessorPrivate::processFlight(Flight flight) const
{
    flight.setDepartureAirport(processAirport(flight.departureAirport()));
    flight.setArrivalAirport(processAirport(flight.arrivalAirport()));
    flight.setAirline(processAirline(flight.airline()));
    flight.setBoardingTime(processFlightTime(flight.boardingTime(), flight, flight.departureAirport()));
    flight.setDepartureTime(processFlightTime(flight.departureTime(), flight, flight.departureAirport()));
    flight.setArrivalTime(processFlightTime(flight.arrivalTime(), flight, flight.arrivalAirport()));
    return flight;
}

Airport ExtractorPostprocessorPrivate::processAirport(Airport airport) const
{
    // clean up name
    airport.setName(airport.name().trimmed());

    // complete missing IATA codes
    auto iataCode = airport.iataCode();
    if (iataCode.isEmpty()) {
        iataCode = KnowledgeDb::iataCodeFromName(airport.name()).toString();
        if (!iataCode.isEmpty()) {
            airport.setIataCode(iataCode);
        }
    }

    // complete missing geo coordinates
    auto geo = airport.geo();
    if (!geo.isValid()) {
        const auto coord = KnowledgeDb::coordinateForAirport(KnowledgeDb::IataCode{iataCode});
        if (coord.isValid()) {
            geo.setLatitude(coord.latitude);
            geo.setLongitude(coord.longitude);
            airport.setGeo(geo);
        }
    }

    // add country
    auto addr = airport.address();
    if (addr.addressCountry().isEmpty()) {
        const auto isoCode = KnowledgeDb::countryForAirport(KnowledgeDb::IataCode{iataCode});
        if (isoCode.isValid()) {
            addr.setAddressCountry(isoCode.toString());
            airport.setAddress(addr);
        }
    }

    return processPlace(airport);
}

Airline ExtractorPostprocessorPrivate::processAirline(Airline airline) const
{
    airline.setName(airline.name().trimmed());
    return airline;
}

QDateTime ExtractorPostprocessorPrivate::processFlightTime(QDateTime dt, const Flight &flight, const Airport &airport) const
{
    if (!dt.isValid()) {
        return dt;
    }

    if (dt.date().year() <= 1970 && flight.departureDay().isValid()) { // we just have the time, but not the day
        dt.setDate(flight.departureDay());
    }

    if (dt.timeSpec() == Qt::TimeZone || airport.iataCode().isEmpty()) {
        return dt;
    }

    const auto tz = KnowledgeDb::timezoneForAirport(KnowledgeDb::IataCode{airport.iataCode()});
    if (!tz.isValid()) {
        return dt;
    }

    // prefer our timezone over externally provided UTC offset, if they match
    if (dt.timeSpec() == Qt::OffsetFromUTC && tz.offsetFromUtc(dt) != dt.offsetFromUtc()) {
        return dt;
    }

    if (dt.timeSpec() == Qt::OffsetFromUTC || dt.timeSpec() == Qt::LocalTime) {
        dt.setTimeSpec(Qt::TimeZone);
        dt.setTimeZone(tz);
    } else if (dt.timeSpec() == Qt::UTC) {
        dt = dt.toTimeZone(tz);
    }

    return dt;
}

TrainReservation ExtractorPostprocessorPrivate::processTrainReservation(TrainReservation res) const
{
    res.setReservationFor(processTrainTrip(res.reservationFor().value<TrainTrip>()));
    return processReservation(res);
}

TrainTrip ExtractorPostprocessorPrivate::processTrainTrip(TrainTrip trip) const
{
    trip.setArrivalPlatform(trip.arrivalPlatform().trimmed());
    trip.setDeparturePlatform(trip.departurePlatform().trimmed());
    trip.setDeparatureStation(processTrainStation(trip.departureStation()));
    trip.setArrivalStation(processTrainStation(trip.arrivalStation()));
    trip.setDepartureTime(processTrainTripTime(trip.departureTime(), trip.departureStation()));
    trip.setArrivalTime(processTrainTripTime(trip.arrivalTime(), trip.arrivalStation()));
    return trip;
}

TrainStation ExtractorPostprocessorPrivate::processTrainStation(TrainStation station) const
{
    const auto id = station.identifier();
    if (id.isEmpty()) { // empty -> null cleanup, to have more compact json-ld output
        station.setIdentifier(QString());
    } else if (id.startsWith(QLatin1String("sncf:")) && id.size() == 10) {
        // Gare & Connexion ids start with a country code, propagate that to the station address field
        auto addr = station.address();
        if (addr.addressCountry().isEmpty()) {
            addr.setAddressCountry(id.mid(5, 2).toUpper());
            station.setAddress(addr);
        }

        const auto record = KnowledgeDb::stationForGaresConnexionsId(KnowledgeDb::GaresConnexionsId{id.mid(5)});
        if (!station.geo().isValid() && record.coordinate.isValid()) {
            GeoCoordinates geo;
            geo.setLatitude(record.coordinate.latitude);
            geo.setLongitude(record.coordinate.longitude);
            station.setGeo(geo);
        }
        if (addr.addressCountry().isEmpty() && record.country.isValid()) {
            addr.setAddressCountry(record.country.toString());
            station.setAddress(addr);
        }
    } else if (id.startsWith(QLatin1String("ibnr:")) && id.size() == 12) {
        const auto record = KnowledgeDb::stationForIbnr(KnowledgeDb::IBNR{id.mid(5).toUInt()});
        if (!station.geo().isValid() && record.coordinate.isValid()) {
            GeoCoordinates geo;
            geo.setLatitude(record.coordinate.latitude);
            geo.setLongitude(record.coordinate.longitude);
            station.setGeo(geo);
        }
        auto addr = station.address();
        if (addr.addressCountry().isEmpty() && record.country.isValid()) {
            addr.setAddressCountry(record.country.toString());
            station.setAddress(addr);
        }
    }

    return processPlace(station);
}

QDateTime ExtractorPostprocessorPrivate::processTrainTripTime(QDateTime dt, const TrainStation& station) const
{
    if (!dt.isValid()) {
        return dt;
    }

    if (dt.timeSpec() == Qt::TimeZone || station.identifier().isEmpty()) {
        return dt;
    }

    QTimeZone tz;
    if (station.identifier().startsWith(QLatin1String("sncf:"))) {
        const auto record = KnowledgeDb::stationForGaresConnexionsId(KnowledgeDb::GaresConnexionsId{station.identifier().mid(5)});
        tz = record.timezone.toQTimeZone();
    } else if (station.identifier().startsWith(QLatin1String("ibnr:"))) {
        const auto record = KnowledgeDb::stationForIbnr(KnowledgeDb::IBNR{station.identifier().mid(5).toUInt()});
        tz = record.timezone.toQTimeZone();
    }
    if (!tz.isValid()) {
        return dt;
    }

    // prefer our timezone over externally provided UTC offset, if they match
    if (dt.timeSpec() == Qt::OffsetFromUTC && tz.offsetFromUtc(dt) != dt.offsetFromUtc()) {
        return dt;
    }

    if (dt.timeSpec() == Qt::OffsetFromUTC || dt.timeSpec() == Qt::LocalTime) {
        dt.setTimeSpec(Qt::TimeZone);
        dt.setTimeZone(tz);
    } else if (dt.timeSpec() == Qt::UTC) {
        dt = dt.toTimeZone(tz);
    }
    return dt;
}

BusReservation ExtractorPostprocessorPrivate::processBusReservation(BusReservation res) const
{
    res.setReservationFor(processBusTrip(res.reservationFor().value<BusTrip>()));
    return processReservation(res);
}

BusTrip ExtractorPostprocessorPrivate::processBusTrip(BusTrip trip) const
{
    trip.setDepartureStation(processPlace(trip.departureStation()));
    trip.setArrivalStation(processPlace(trip.arrivalStation()));
    return trip;
}

LodgingReservation ExtractorPostprocessorPrivate::processLodgingReservation(LodgingReservation res) const
{
    res.setReservationFor(processPlace(res.reservationFor().value<LodgingBusiness>()));
    return processReservation(res);
}

RentalCarReservation ExtractorPostprocessorPrivate::processRentalCarReservation(RentalCarReservation res) const
{
    res.setPickUpLocation(processPlace(res.pickUpLocation()));
    res.setDropOffLocation(processPlace(res.dropOffLocation()));
    return processReservation(res);
}

FoodEstablishmentReservation ExtractorPostprocessorPrivate::processFoodEstablishmentReservation(FoodEstablishmentReservation res) const
{
    res.setReservationFor(processPlace(res.reservationFor().value<FoodEstablishment>()));
    return processReservation(res);
}

TouristAttractionVisit ExtractorPostprocessorPrivate::processTouristAttractionVisit(TouristAttractionVisit visit) const
{
    visit.setTouristAttraction(processPlace(visit.touristAttraction()));
    return visit;
}

EventReservation ExtractorPostprocessorPrivate::processEventReservation(EventReservation res) const
{
    res.setReservationFor(processEvent(res.reservationFor().value<Event>()));
    return processReservation(res);
}

Event ExtractorPostprocessorPrivate::processEvent(Event event) const
{
    // normalize location to be a Place
    if (JsonLd::isA<PostalAddress>(event.location())) {
        Place place;
        place.setAddress(event.location().value<PostalAddress>());
        event.setLocation(place);
    }

    if (JsonLd::isA<Place>(event.location())) {
        event.setLocation(processPlace(event.location().value<Place>()));
    }
    return event;
}

template <typename T>
T ExtractorPostprocessorPrivate::processReservation(T res) const
{
    res.setUnderName(processPerson(res.underName().template value<Person>()));
    res.setPotentialAction(processActions(res.potentialAction()));
    return res;
}


Person ExtractorPostprocessorPrivate::processPerson(Person person) const
{
    person.setName(person.name().simplified());

    if (person.name().isEmpty() && !person.familyName().isEmpty() && !person.givenName().isEmpty()) {
        person.setName(person.givenName() + QLatin1Char(' ') + person.familyName());
    }

    // strip prefixes, they break comparisons
    static const char* honorificPrefixes[] = { "MR ", "MS ", "MRS " };
    for (auto prefix : honorificPrefixes) {
        if (person.name().startsWith(QLatin1String(prefix), Qt::CaseInsensitive)) {
            person.setName(person.name().mid(strlen(prefix)));
            break;
        }
    }

    return person;
}

template<typename T> T ExtractorPostprocessorPrivate::processPlace(T place) const
{
#ifdef HAVE_KCONTACTS
    auto addr = place.address();
    if (!addr.addressCountry().isEmpty() && addr.addressCountry().size() != 2) {
        const auto isoCode = KContacts::Address::countryToISO(addr.addressCountry()).toUpper();
        if (!isoCode.isEmpty()) {
            addr.setAddressCountry(isoCode);
            place.setAddress(addr);
        }
    }
#endif
    return place;
}

QVariantList ExtractorPostprocessorPrivate::processActions(QVariantList actions) const
{
    // remove non-actions and actions with invalid URLs
    QUrl viewUrl;
    for (auto it = actions.begin(); it != actions.end();) {
        if (!JsonLd::canConvert<Action>(*it)) {
            it = actions.erase(it);
            continue;
        }

        const auto action = JsonLd::convert<Action>(*it);
        if (!action.target().isValid()) {
            it = actions.erase(it);
            continue;
        }

        if (JsonLd::isA<ViewAction>(*it)) {
            viewUrl = action.target();
        }
        ++it;
    }

    // normalize the order, so JSON comparisson still yields correct results
    std::sort(actions.begin(), actions.end(), [](const QVariant &lhs, const QVariant &rhs) {
        return strcmp(lhs.typeName(), rhs.typeName()) < 0;
    });

    // remove actions that don't actually have their own target, or duplicates
    QUrl prevUrl;
    const char* prevType = nullptr;
    for (auto it = actions.begin(); it != actions.end();) {
        const auto action = JsonLd::convert<Action>(*it);
        const auto isDuplicate = action.target() == prevUrl && (prevType ? strcmp(prevType, (*it).typeName()) == 0 : false);
        if ((JsonLd::isA<ViewAction>(*it) || action.target() != viewUrl) && !isDuplicate) {
            prevUrl = action.target();
            prevType = (*it).typeName();
            ++it;
        } else {
            it = actions.erase(it);
        }
    }

    return actions;
}

bool ExtractorPostprocessorPrivate::filterReservation(const QVariant &res) const
{
    if (JsonLd::isA<FlightReservation>(res)) {
        return filterFlight(res.value<FlightReservation>().reservationFor().value<Flight>());
    }
    if (JsonLd::isA<TrainReservation>(res)) {
        return filterTrainOrBusTrip(res.value<TrainReservation>().reservationFor().value<TrainTrip>());
    }
    if (JsonLd::isA<BusReservation>(res)) {
        return filterTrainOrBusTrip(res.value<BusReservation>().reservationFor().value<BusTrip>());
    }
    if (JsonLd::isA<LodgingReservation>(res)) {
        return filterLodgingReservation(res.value<LodgingReservation>());
    }
    if (JsonLd::isA<EventReservation>(res)) {
        return filterEventReservation(res.value<EventReservation>());
    }

    // types without specific filters yet
    if (JsonLd::isA<TouristAttractionVisit>(res) || JsonLd::isA<FoodEstablishmentReservation>(res) || JsonLd::isA<RentalCarReservation>(res)) {
        return true;
    }

    // unknown top-level type
    return false;
}

bool ExtractorPostprocessorPrivate::filterLodgingReservation(const LodgingReservation &res) const
{
    return res.checkinTime().isValid() && res.checkoutTime().isValid();
}

bool ExtractorPostprocessorPrivate::filterFlight(const Flight &flight) const
{
    // this will be valid if either boarding time, departure time or departure day is set
    const auto validDate = flight.departureDay().isValid();
    return filterAirport(flight.departureAirport())
           && filterAirport(flight.arrivalAirport())
           && validDate;
}

bool ExtractorPostprocessorPrivate::filterAirport(const Airport &airport) const
{
    return !airport.iataCode().isEmpty() || !airport.name().isEmpty();
}

template <typename T>
bool ExtractorPostprocessorPrivate::filterTrainOrBusTrip(const T &trip) const
{
    return filterTrainOrBusStation(trip.departureStation())
           && filterTrainOrBusStation(trip.arrivalStation())
           && trip.departureTime().isValid() && trip.arrivalTime().isValid();
}

template <typename T>
bool ExtractorPostprocessorPrivate::filterTrainOrBusStation(const T &station) const
{
    return !station.name().isEmpty();
}

bool ExtractorPostprocessorPrivate::filterEventReservation(const EventReservation &res) const
{
    const auto event = res.reservationFor().value<Event>();
    return !event.name().isEmpty() && event.startDate().isValid();
}
