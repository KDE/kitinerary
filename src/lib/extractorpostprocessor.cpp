/*
   SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "config-kitinerary.h"
#include "extractorpostprocessor.h"
#include "extractorpostprocessor_p.h"
#include "extractorvalidator.h"
#include "flightpostprocessor_p.h"
#include "stringutil.h"

#include "iata/iatabcbpparser.h"
#include "jsonlddocument.h"
#include "logging.h"
#include "mergeutil.h"
#include "sortutil.h"
#include "text/addressparser_p.h"

#include "knowledgedb/airportdb.h"
#include "knowledgedb/timezonedb_p.h"
#include "knowledgedb/trainstationdb.h"

#include <KItinerary/Action>
#include <KItinerary/BoatTrip>
#include <KItinerary/BusTrip>
#include <KItinerary/Event>
#include <KItinerary/Flight>
#include <KItinerary/Organization>
#include <KItinerary/Person>
#include <KItinerary/Place>
#include <KItinerary/ProgramMembership>
#include <KItinerary/RentalCar>
#include <KItinerary/Reservation>
#include <KItinerary/Taxi>
#include <KItinerary/Ticket>
#include <KItinerary/TrainTrip>
#include <KItinerary/Visit>

#include <KCountry>

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTimeZone>
#include <QUrl>

#if HAVE_PHONENUMBER
#include <phonenumbers/phonenumberutil.h>
#endif

#include <algorithm>
#include <cstring>

using namespace Qt::Literals::StringLiterals;
using namespace KItinerary;

ExtractorPostprocessor::ExtractorPostprocessor()
    : d(new ExtractorPostprocessorPrivate)
{
}

ExtractorPostprocessor::ExtractorPostprocessor(ExtractorPostprocessor &&) noexcept = default;
ExtractorPostprocessor::~ExtractorPostprocessor() = default;

void ExtractorPostprocessor::process(const QList<QVariant> &data) {
    d->m_resultFinalized = false;
    d->m_data.reserve(d->m_data.size() + data.size());
    for (auto elem : data) {
        // reservation types
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
        } else if (JsonLd::isA<BoatReservation>(elem)) {
            elem = d->processBoatReservation(elem.value<BoatReservation>());
        } else if (JsonLd::isA<EventReservation>(elem)) {
            elem = d->processEventReservation(elem.value<EventReservation>());
        } else if (JsonLd::isA<RentalCarReservation>(elem)) {
            elem = d->processRentalCarReservation(elem.value<RentalCarReservation>());
        } else if (JsonLd::isA<TaxiReservation>(elem)) {
            elem = d->processTaxiReservation(elem.value<TaxiReservation>());
        }

        // "reservationFor" types
        else if (JsonLd::isA<LodgingBusiness>(elem)) {
            elem = d->processPlace(elem.value<LodgingBusiness>());
        } else if (JsonLd::isA<FoodEstablishment>(elem)) {
            elem = d->processPlace(elem.value<FoodEstablishment>());
        } else if (JsonLd::isA<Event>(elem)) {
            elem = d->processEvent(elem.value<Event>());
        } else if (JsonLd::isA<LocalBusiness>(elem)) {
            elem = d->processPlace(elem.value<LocalBusiness>());
        }

        // non-reservation types
        else if (JsonLd::isA<ProgramMembership>(elem)) {
            elem = d->processProgramMembership(elem.value<ProgramMembership>());
        } else if (JsonLd::isA<Ticket>(elem)) {
            elem = d->processTicket(elem.value<Ticket>());
        }

        d->mergeOrAppend(elem);
    }
}

[[nodiscard]] static QVariant mergeTicket(QVariant lhs, const QVariant &rhs)
{
    const auto rhsTicket = JsonLdDocument::readProperty(rhs, "reservedTicket");
    const auto lhsTicket = JsonLdDocument::readProperty(lhs, "reservedTicket");
    JsonLdDocument::writeProperty(lhs, "reservedTicket", MergeUtil::merge(lhsTicket, rhsTicket));
    return lhs;
}

QList<QVariant> ExtractorPostprocessor::result() const {
    if (!d->m_resultFinalized) {
        // fold elements we have reservations for into those reservations
        for (auto it = d->m_data.begin(); it != d->m_data.end();) {
            if (JsonLd::isA<Reservation>(*it)) {
                ++it;
                continue;
            }

            bool merged = false;
            for (auto it2 = d->m_data.begin(); it2 != d->m_data.end(); ++it2) {
                const auto resFor = JsonLdDocument::readProperty(*it2, "reservationFor");
                if (MergeUtil::isSame(resFor, *it)) {
                    JsonLdDocument::writeProperty(*it2, "reservationFor", MergeUtil::merge(resFor, *it));
                    merged = true;
                }
            }

            if (merged) {
                it = d->m_data.erase(it);
            } else {
                ++it;
            }
        }

        // search for "triangular" patterns, ie. a location change element that has a matching departure
        // and matching arrival to two different other location change elements (A->C vs A->B + B->C).
        // we remove those, as the fine-granular results are better
        if (d->m_data.size() >= 3) {
            for (auto it = d->m_data.begin(); it != d->m_data.end();) {
                auto depIt = it;
                auto arrIt = it;
                for (auto it2 = d->m_data.begin(); it2 != d->m_data.end(); ++it2) {
                    if (it == it2) {
                        continue;
                    }
                    if (MergeUtil::hasSameDeparture(*it, *it2)) {
                        depIt = it2;
                    }
                    if (MergeUtil::hasSameArrival(*it, *it2)) {
                        arrIt = it2;
                    }
                }

                if (depIt != it && arrIt != it && depIt != arrIt) {
                    (*depIt) = mergeTicket(*depIt, *it);
                    (*arrIt) = mergeTicket(*arrIt, *it);
                    it = d->m_data.erase(it);
                } else {
                    ++it;
                }
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

void ExtractorPostprocessor::setValidationEnabled([[maybe_unused]] bool validate)
{
}

void ExtractorPostprocessorPrivate::mergeOrAppend(const QVariant &elem)
{
    const auto it = std::find_if(m_data.begin(), m_data.end(), [elem](const QVariant &other) {
        return MergeUtil::isSame(elem, other);
    });

    if (it == m_data.end()) {
        m_data.push_back(elem);
    } else {
        *it = MergeUtil::merge(*it, elem);
    }
}

QVariant ExtractorPostprocessorPrivate::processFlightReservation(FlightReservation res) const
{
    // expand ticketToken for IATA BCBP data
    const auto bcbp = res.reservedTicket().value<Ticket>().ticketTokenData().toString();
    if (!bcbp.isEmpty()) {
        const auto bcbpData = IataBcbpParser::parse(bcbp, m_contextDate);
        if (bcbpData.size() == 1) {
            res = JsonLdDocument::apply(bcbpData.at(0), res).value<FlightReservation>();
            // standardize on the BCBP booking reference, not some secondary one we might have in structured data for example
            res.setReservationNumber(bcbpData.at(0).value<FlightReservation>().reservationNumber());
        } else {
            for (const auto &data : bcbpData) {
                if (MergeUtil::isSame(res, data)) {
                    res = JsonLdDocument::apply(data, res).value<FlightReservation>();
                    break;
                }
            }
        }
    }

    res.setBoardingGroup(StringUtil::simplifiedNoPlaceholder(res.boardingGroup()));
    if (res.reservationFor().isValid()) {
        FlightPostProcessor p;
        res.setReservationFor(p.processFlight(res.reservationFor().value<Flight>()));
    }
    return processReservation(res);
}

TrainReservation ExtractorPostprocessorPrivate::processTrainReservation(TrainReservation res) const
{
    if (res.reservationFor().isValid()) {
        res.setReservationFor(processTrainTrip(res.reservationFor().value<TrainTrip>()));
    }
    return processReservation(res);
}

TrainTrip ExtractorPostprocessorPrivate::processTrainTrip(TrainTrip trip) const
{
    trip.setArrivalPlatform(trip.arrivalPlatform().trimmed());
    trip.setDeparturePlatform(trip.departurePlatform().trimmed());
    trip.setDepartureStation(processTrainStation(trip.departureStation()));
    trip.setArrivalStation(processTrainStation(trip.arrivalStation()));
    trip.setDepartureTime(processTrainTripTime(trip.departureTime(), trip.departureDay(), trip.departureStation()));
    trip.setArrivalTime(processTrainTripTime(trip.arrivalTime(), trip.departureDay(), trip.arrivalStation()));
    trip.setTrainNumber(trip.trainNumber().simplified());
    trip.setTrainName(trip.trainName().simplified());

    // arrival less than a day before departure is an indication of the extractor failing to detect day rollover
    const auto duration = trip.departureTime().secsTo(trip.arrivalTime());
    if (duration < 0 && duration > -3600*24 && trip.departureTime().timeSpec() == trip.arrivalTime().timeSpec()) {
        trip.setArrivalTime(trip.arrivalTime().addDays(1));
    }

    return trip;
}

static void applyStationData(const KnowledgeDb::TrainStation &record, TrainStation &station)
{
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

static void applyStationCountry(const QString &isoCode, TrainStation &station)
{
    auto addr = station.address();
    if (addr.addressCountry().isEmpty()) {
        addr.setAddressCountry(isoCode.toUpper());
        station.setAddress(addr);
    }
}

TrainStation ExtractorPostprocessorPrivate::processTrainStation(TrainStation station) const
{
    const auto id = station.identifier();
    if (id.isEmpty()) { // empty -> null cleanup, to have more compact json-ld output
        station.setIdentifier(QString());
    } else if (id.startsWith("sncf:"_L1) && id.size() == 10) {
        const auto record = KnowledgeDb::stationForSncfStationId(KnowledgeDb::SncfStationId{id.mid(5)});
        applyStationData(record, station);
        applyStationCountry(id.mid(5, 2).toUpper(), station);
    } else if (id.startsWith("ibnr:"_L1) && id.size() == 12) {
        const auto record = KnowledgeDb::stationForIbnr(KnowledgeDb::IBNR{id.mid(5).toUInt()});
        applyStationData(record, station);
        const auto country = KnowledgeDb::countryIdForUicCode(QStringView(id).mid(5, 2).toUShort()) .toString();
      applyStationCountry(country, station);
    } else if (id.startsWith("uic:"_L1) && id.size() == 11) {
        const auto record = KnowledgeDb::stationForUic( KnowledgeDb::UICStation{id.mid(4).toUInt()});
        applyStationData(record, station);
        const auto country = KnowledgeDb::countryIdForUicCode(QStringView(id).mid(4, 2).toUShort()) .toString();
        applyStationCountry(country, station);
    } else if (id.startsWith("ir:"_L1) && id.size() > 4) {
        const auto record = KnowledgeDb::stationForIndianRailwaysStationCode(id.mid(3));
        applyStationData(record, station);
    } else if (id.startsWith("benerail:"_L1) && id.size() == 14) {
        const auto record = KnowledgeDb::stationForBenerailId(KnowledgeDb::BenerailStationId(id.mid(9)));
        applyStationData(record, station);
        applyStationCountry(id.mid(9, 2).toUpper(), station);
    } else if (id.startsWith("vrfi:"_L1) && id.size() >= 7 && id.size() <= 9) {
        const auto record = KnowledgeDb::stationForVRStationCode(KnowledgeDb::VRStationCode(id.mid(5)));
        applyStationData(record, station);
    } else if (id.startsWith("iata:"_L1) && id.size() == 8) {
        const auto iataCode = KnowledgeDb::IataCode(QStringView(id).mid(5));
        const auto record = KnowledgeDb::stationForIataCode(iataCode);
        applyStationData(record, station);
        // fall back to the airport with the matching IATA code for the country
        // information we cannot use the coordinate though, as that points to the
        // actual airport, not the station
        applyStationCountry(KnowledgeDb::countryForAirport(iataCode).toString(), station);
    } else if (id.startsWith("amtrak:"_L1) && id.size() == 10) {
        const auto record = KnowledgeDb::stationForAmtrakStationCode(KnowledgeDb::AmtrakStationCode(QStringView(id).mid(7)));
        applyStationData(record, station);
    } else if (id.startsWith("via:"_L1) && id.size() == 8) {
        const auto record = KnowledgeDb::stationForViaRailStationCode(KnowledgeDb::ViaRailStationCode(QStringView(id).mid(4)));
        applyStationData(record, station);
    } else if (id.startsWith("uk:"_L1) && id.size() == 6) {
        const auto record = KnowledgeDb::stationForUkRailwayStationCode(KnowledgeDb::UKRailwayStationCode(QStringView(id).mid(3)));
        applyStationData(record, station);
    }

    return processPlace(station);
}

QDateTime ExtractorPostprocessorPrivate::processTrainTripTime(QDateTime dt, QDate departureDay, const TrainStation& station) const
{
    if (!dt.isValid()) {
        return dt;
    }

    if (dt.date().year() <= 1970 && departureDay.isValid()) { // we just have the time, but not the day
        dt.setDate(departureDay);
    }
    return processTimeForLocation(dt, station);
}

BusReservation ExtractorPostprocessorPrivate::processBusReservation(BusReservation res) const
{
    if (res.reservationFor().isValid()) {
        res.setReservationFor(processBusTrip(res.reservationFor().value<BusTrip>()));
    }
    return processReservation(res);
}

BusTrip ExtractorPostprocessorPrivate::processBusTrip(BusTrip trip) const
{
    trip.setDepartureBusStop(processPlace(trip.departureBusStop()));
    trip.setArrivalBusStop(processPlace(trip.arrivalBusStop()));
    trip.setDepartureTime(processTimeForLocation(trip.departureTime(), trip.departureBusStop()));
    trip.setArrivalTime(processTimeForLocation(trip.arrivalTime(), trip.arrivalBusStop()));
    trip.setBusNumber(trip.busNumber().simplified());
    trip.setBusName(trip.busName().simplified());
    return trip;
}

BoatReservation ExtractorPostprocessorPrivate::processBoatReservation(BoatReservation res) const
{
    if (res.reservationFor().isValid()) {
        res.setReservationFor(processBoatTrip(res.reservationFor().value<BoatTrip>()));
    }
    return processReservation(res);
}

BoatTrip ExtractorPostprocessorPrivate::processBoatTrip(BoatTrip trip) const
{
    trip.setDepartureBoatTerminal(processPlace(trip.departureBoatTerminal()));
    trip.setArrivalBoatTerminal(processPlace(trip.arrivalBoatTerminal()));
    trip.setDepartureTime(processTimeForLocation(trip.departureTime(), trip.departureBoatTerminal()));
    trip.setArrivalTime(processTimeForLocation(trip.arrivalTime(), trip.arrivalBoatTerminal()));

    // arrival less than a day before departure is an indication of the extractor failing to detect day rollover
    const auto duration = trip.departureTime().secsTo(trip.arrivalTime());
    if (duration < 0 && duration > -3600*24) {
        trip.setArrivalTime(trip.arrivalTime().addDays(1));
    }

    return trip;
}

LodgingReservation ExtractorPostprocessorPrivate::processLodgingReservation(LodgingReservation res) const
{
    if (res.reservationFor().isValid()) {
        res.setReservationFor(processPlace(res.reservationFor().value<LodgingBusiness>()));
        res.setCheckinTime(processTimeForLocation(res.checkinTime(), res.reservationFor().value<LodgingBusiness>()));
        res.setCheckoutTime(processTimeForLocation(res.checkoutTime(), res.reservationFor().value<LodgingBusiness>()));
    }
    return processReservation(res);
}

TaxiReservation ExtractorPostprocessorPrivate::processTaxiReservation(TaxiReservation res) const
{
    res.setPickupLocation(processPlace(res.pickupLocation()));
    res.setPickupTime(processTimeForLocation(res.pickupTime(), res.pickupLocation()));
    return processReservation(res);
}

RentalCarReservation ExtractorPostprocessorPrivate::processRentalCarReservation(RentalCarReservation res) const
{
    if (res.reservationFor().isValid()) {
        res.setReservationFor(processRentalCar(res.reservationFor().value<RentalCar>()));
    }
    res.setPickupLocation(processPlace(res.pickupLocation()));
    res.setDropoffLocation(processPlace(res.dropoffLocation()));
    res.setPickupTime(processTimeForLocation(res.pickupTime(), res.pickupLocation()));
    res.setDropoffTime(processTimeForLocation(res.dropoffTime(), res.dropoffLocation()));
    return processReservation(res);
}

RentalCar ExtractorPostprocessorPrivate::processRentalCar(RentalCar car) const
{
    car.setName(car.name().trimmed());
    return car;
}

FoodEstablishmentReservation ExtractorPostprocessorPrivate::processFoodEstablishmentReservation(FoodEstablishmentReservation res) const
{
    if (res.reservationFor().isValid()) {
        res.setReservationFor(processPlace(res.reservationFor().value<FoodEstablishment>()));
        res.setStartTime(processTimeForLocation(res.startTime(), res.reservationFor().value<FoodEstablishment>()));
        res.setEndTime(processTimeForLocation(res.endTime(), res.reservationFor().value<FoodEstablishment>()));
    }
    return processReservation(res);
}

TouristAttractionVisit ExtractorPostprocessorPrivate::processTouristAttractionVisit(TouristAttractionVisit visit) const
{
    visit.setTouristAttraction(processPlace(visit.touristAttraction()));
    visit.setArrivalTime(processTimeForLocation(visit.arrivalTime(), visit.touristAttraction()));
    visit.setDepartureTime(processTimeForLocation(visit.departureTime(), visit.touristAttraction()));
    return visit;
}

EventReservation ExtractorPostprocessorPrivate::processEventReservation(EventReservation res) const
{
    if (res.reservationFor().isValid()) {
        res.setReservationFor(processEvent(res.reservationFor().value<Event>()));
    }
    return processReservation(res);
}

KItinerary::Event ExtractorPostprocessorPrivate::processEvent(KItinerary::Event event) const
{
    event.setName(StringUtil::clean(event.name()));

    // normalize location to be a Place
    if (JsonLd::isA<PostalAddress>(event.location())) {
        Place place;
        place.setAddress(event.location().value<PostalAddress>());
        event.setLocation(place);
    }

    if (JsonLd::isA<Place>(event.location())) {
        event.setLocation(processPlace(event.location().value<Place>()));

        // try to obtain timezones if we have a location
        event.setStartDate(processTimeForLocation(event.startDate(), event.location().value<Place>()));
        event.setEndDate(processTimeForLocation(event.endDate(), event.location().value<Place>()));
        event.setDoorTime(processTimeForLocation(event.doorTime(), event.location().value<Place>()));
    }

    return event;
}

static QString processCurrency(const QString &currency)
{
    if (currency.size() != 3 || !std::all_of(currency.begin(), currency.end(), [](QChar c) { return c.isUpper(); })) {
        return {};
    }
    return currency;
}

Ticket ExtractorPostprocessorPrivate::processTicket(Ticket ticket) const
{
    ticket.setName(StringUtil::clean(ticket.name()));
    ticket.setTicketNumber(ticket.ticketNumber().simplified());
    ticket.setUnderName(processPerson(ticket.underName()));
    ticket.setTicketedSeat(processSeat(ticket.ticketedSeat()));
    ticket.setPriceCurrency(processCurrency(ticket.priceCurrency()));
    return ticket;
}

ProgramMembership ExtractorPostprocessorPrivate::processProgramMembership(ProgramMembership program) const
{
    // remove empty dummy entries found eg. in ERA FCB data
    if (const auto name = program.programName(); std::none_of(name.begin(), name.end(), [](QChar c) { return c.isLetter(); })) {
        program.setProgramName(QString());
    }

    program.setProgramName(program.programName().simplified());
    // avoid emitting spurious empty ProgramMembership objects caused by empty elements in JSON-LD/Microdata input
    if (program.programName().isEmpty() && !program.programName().isNull()) {
        program.setProgramName(QString());
    }
    program.setMember(processPerson(program.member()));
    return program;
}

Seat ExtractorPostprocessorPrivate::processSeat(Seat seat) const
{
    seat.setSeatSection(seat.seatSection().simplified());
    seat.setSeatRow(seat.seatRow().simplified());
    seat.setSeatNumber(seat.seatNumber().simplified());
    seat.setSeatingType(seat.seatingType().simplified());
    return seat;
}

template <typename T>
T ExtractorPostprocessorPrivate::processReservation(T res) const
{
    res.setUnderName(processPerson(res.underName().template value<Person>()));
    res.setPotentialAction(processActions(res.potentialAction()));
    res.setReservationNumber(res.reservationNumber().trimmed());
    res.setProgramMembershipUsed(processProgramMembership(res.programMembershipUsed()));
    res.setPriceCurrency(processCurrency(res.priceCurrency()));

    if (JsonLd::isA<Ticket>(res.reservedTicket())) {
        res.setReservedTicket(processTicket(res.reservedTicket().template value<Ticket>()));
    }
    return res;
}

static constexpr const char* name_prefixes[] = {
    "DR", "MR", "MRS", "MS"
};

static bool isSeparator(QChar c)
{
    return c == QLatin1Char(' ') || c == QLatin1Char('/');
}

static QString simplifyNamePart(QString n)
{
    n = n.simplified();

    for (auto prefix : name_prefixes) {
        const int prefixLen = std::strlen(prefix);
        if (n.size() > prefixLen + 2 &&
            n.startsWith(QLatin1StringView(prefix, prefixLen),
                         Qt::CaseInsensitive) &&
            isSeparator(n[prefixLen])) {
          return n.mid(prefixLen + 1);
        }
        if (n.size() > prefixLen + 2 &&
            n.endsWith(QLatin1StringView(prefix, prefixLen),
                       Qt::CaseInsensitive) &&
            isSeparator(n[n.size() - prefixLen - 1])) {
          return n.left(n.size() - prefixLen - 1);
        }
    }

    return n;
}

KItinerary::Person ExtractorPostprocessorPrivate::processPerson(KItinerary::Person person) const
{
    person.setName(simplifyNamePart(person.name()));
    person.setFamilyName(simplifyNamePart(person.familyName()));
    person.setGivenName(simplifyNamePart(person.givenName()));

    // fill name with name parts, if it's empty
    if ((person.name().isEmpty() || person.name() == person.familyName() || person.name() == person.givenName())
        && !person.familyName().isEmpty() && !person.givenName().isEmpty())
    {
        person.setName(person.givenName() + QLatin1Char(' ') + person.familyName());
    }

    return person;
}

PostalAddress ExtractorPostprocessorPrivate::processAddress(PostalAddress addr, const QString &phoneNumber, const GeoCoordinates &geo)
{
    addr.setAddressCountry(addr.addressCountry().simplified());

    // convert to ISO 3166-1 alpha-2 country codes
    if (addr.addressCountry().size() > 2) {
        QString alpha2Code;

        // try ISO 3166-1 alpha-3, we get that e.g. from Flixbus
        if (addr.addressCountry().size() == 3) {
            alpha2Code = KCountry::fromAlpha3(addr.addressCountry()).alpha2();
        }
        if (alpha2Code.isEmpty()) {
            alpha2Code = KCountry::fromName(addr.addressCountry()).alpha2();
        }
        if (!alpha2Code.isEmpty()) {
            addr.setAddressCountry(alpha2Code);
        }
    }

    // upper case country codes
    if (addr.addressCountry().size() == 2) {
        addr.setAddressCountry(addr.addressCountry().toUpper());
    }

    // normalize strings
    addr.setStreetAddress(addr.streetAddress().simplified());
    addr.setPostalCode(addr.postalCode().simplified());
    addr.setAddressLocality(addr.addressLocality().simplified());
    addr.setAddressRegion(addr.addressRegion().simplified());

#if HAVE_PHONENUMBER
    // recover country from phone number, if we have that
    if (!phoneNumber.isEmpty() && addr.addressCountry().size() != 2) {
        const auto phoneStr = phoneNumber.toStdString();
        const auto util = i18n::phonenumbers::PhoneNumberUtil::GetInstance();
        i18n::phonenumbers::PhoneNumber number;
        if (util->ParseAndKeepRawInput(phoneStr, "ZZ", &number) == i18n::phonenumbers::PhoneNumberUtil::NO_PARSING_ERROR) {
            std::string isoCode;
            util->GetRegionCodeForNumber(number, &isoCode);
            if (!isoCode.empty() && isoCode != "ZZ") {
                addr.setAddressCountry(QString::fromStdString(isoCode));
            }
        }
    }
#endif

    if (geo.isValid() && addr.addressCountry().size() != 2) {
        const auto country = KCountry::fromLocation(geo.latitude(), geo.longitude());
        if (country.isValid()) {
            addr.setAddressCountry(country.alpha2());
        }
    }

    AddressParser addrParser;
    addrParser.setFallbackCountry(KCountry::fromQLocale(QLocale().territory()).alpha2());
    addrParser.parse(addr);
    addr = addrParser.result();
    return addr;
}

QString ExtractorPostprocessorPrivate::processPhoneNumber(const QString &phoneNumber, const PostalAddress &addr)
{
#if HAVE_PHONENUMBER
    // or complete the phone number if we know the country
    if (!phoneNumber.isEmpty() && addr.addressCountry().size() == 2) {
        auto phoneStr = phoneNumber.toStdString();
        const auto isoCode = addr.addressCountry().toStdString();
        const auto util = i18n::phonenumbers::PhoneNumberUtil::GetInstance();
        i18n::phonenumbers::PhoneNumber number;
        if (util->ParseAndKeepRawInput(phoneStr, isoCode, &number) == i18n::phonenumbers::PhoneNumberUtil::NO_PARSING_ERROR) {
            if (number.country_code_source() == i18n::phonenumbers::PhoneNumber_CountryCodeSource_FROM_DEFAULT_COUNTRY) {
                util->Format(number, i18n::phonenumbers::PhoneNumberUtil::INTERNATIONAL, &phoneStr);
                return QString::fromStdString(phoneStr);
            }
        }
    }
#else
    Q_UNUSED(addr)
#endif
    return phoneNumber.simplified();
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

    // normalize the order, so JSON comparison still yields correct results
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

template <typename T>
QDateTime ExtractorPostprocessorPrivate::processTimeForLocation(QDateTime dt, const T &place) const
{
    if (!dt.isValid() ) {
        return dt;
    }
    if ((dt.timeSpec() == Qt::TimeZone && dt.timeZone() != QTimeZone::utc())) {
        if (KnowledgeDb::isPlausibleTimeZone(dt.timeZone(), place.geo().latitude(), place.geo().longitude(), place.address().addressCountry(), place.address().addressRegion())) {
            return dt;
        }
        // drop timezones where we are sure they don't match the location
        dt.setTimeZone(QTimeZone::LocalTime);
    }

    const auto tz = KnowledgeDb::timezoneForLocation(place.geo().latitude(), place.geo().longitude(), place.address().addressCountry(), place.address().addressRegion());
    if (!tz.isValid()) {
        return dt;
    }

    // prefer our timezone over externally provided UTC offset, if they match
    if (dt.timeSpec() == Qt::OffsetFromUTC && tz.offsetFromUtc(dt) != dt.offsetFromUtc()) {
        qCDebug(Log) << "UTC offset clashes with expected timezone!" << dt << dt.offsetFromUtc() << tz.id() << tz.offsetFromUtc(dt);
        return dt;
    }

    if (dt.timeSpec() == Qt::OffsetFromUTC || dt.timeSpec() == Qt::LocalTime) {
        dt.setTimeZone(tz);
    } else if (dt.timeSpec() == Qt::UTC || (dt.timeSpec() == Qt::TimeZone && dt.timeZone() == QTimeZone::utc())) {
        dt = dt.toTimeZone(tz);
    }
    return dt;
}
