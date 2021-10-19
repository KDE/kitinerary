/*
   SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "config-kitinerary.h"
#include "extractorpostprocessor.h"
#include "extractorpostprocessor_p.h"
#include "extractorvalidator.h"
#include "flightpostprocessor_p.h"

#include "extractorutil.h"
#include "iata/iatabcbpparser.h"
#include "jsonlddocument.h"
#include "logging.h"
#include "mergeutil.h"
#include "sortutil.h"

#include "knowledgedb/trainstationdb.h"

#include <KItinerary/Action>
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

#if HAVE_KI18N_LOCALE_DATA
#include <KCountry>
#else
#include <KContacts/Address>
#endif

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTimeZone>
#include <QUrl>

#ifdef HAVE_PHONENUMBER
#include <phonenumbers/phonenumberutil.h>
#endif

#include <algorithm>

using namespace KItinerary;

ExtractorPostprocessor::ExtractorPostprocessor()
    : d(new ExtractorPostprocessorPrivate)
{
    // configure the default set of accepted types, for backward compatibility
    d->m_validator.setAcceptedTypes<
        FlightReservation,
        TrainReservation,
        BusReservation,
        RentalCarReservation,
        TaxiReservation,
        EventReservation,
        FoodEstablishmentReservation,
        LodgingReservation,
        // reservationFor types
        Flight,
        TrainTrip,
        BusTrip,
        RentalCar,
        Taxi,
        Event,
        TouristAttractionVisit,
        FoodEstablishment,
        // PBI types
        LocalBusiness
    >();
}

ExtractorPostprocessor::ExtractorPostprocessor(ExtractorPostprocessor &&) noexcept = default;
ExtractorPostprocessor::~ExtractorPostprocessor() = default;

void ExtractorPostprocessor::process(const QVector<QVariant> &data)
{
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
        }

        d->mergeOrAppend(elem);
    }
}

QVector<QVariant> ExtractorPostprocessor::result() const
{
    if (!d->m_resultFinalized && d->m_validationEnabled) {
        d->m_data.erase(std::remove_if(d->m_data.begin(), d->m_data.end(), [this](const auto &elem) {
            return !d->m_validator.isValidElement(elem);
        }), d->m_data.end());
        d->m_resultFinalized = true;
    }

    std::stable_sort(d->m_data.begin(), d->m_data.end(), SortUtil::isBefore);
    return d->m_data;
}

void ExtractorPostprocessor::setContextDate(const QDateTime& dt)
{
    d->m_contextDate = dt;
}

void ExtractorPostprocessor::setValidationEnabled(bool validate)
{
    d->m_validationEnabled = validate;
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
    } else if (id.startsWith(QLatin1String("sncf:")) && id.size() == 10) {
        const auto record = KnowledgeDb::stationForSncfStationId(KnowledgeDb::SncfStationId{id.mid(5)});
        applyStationData(record, station);
        applyStationCountry(id.mid(5, 2).toUpper(), station);
    } else if (id.startsWith(QLatin1String("ibnr:")) && id.size() == 12) {
        const auto record = KnowledgeDb::stationForIbnr(KnowledgeDb::IBNR{id.mid(5).toUInt()});
        applyStationData(record, station);
        const auto country = KnowledgeDb::countryIdForUicCode(id.midRef(5, 2).toUShort()).toString();
        applyStationCountry(country, station);
    } else if (id.startsWith(QLatin1String("uic:")) && id.size() == 11) {
        const auto record = KnowledgeDb::stationForUic(KnowledgeDb::UICStation{id.mid(4).toUInt()});
        applyStationData(record, station);
        const auto country = KnowledgeDb::countryIdForUicCode(id.midRef(4, 2).toUShort()).toString();
        applyStationCountry(country, station);
    } else if (id.startsWith(QLatin1String("ir:")) && id.size() > 4) {
        const auto record = KnowledgeDb::stationForIndianRailwaysStationCode(id.mid(3));
        applyStationData(record, station);
    } else if (id.startsWith(QLatin1String("benerail:")) && id.size() == 14) {
        const auto record = KnowledgeDb::stationForBenerailId(KnowledgeDb::BenerailStationId(id.mid(9)));
        applyStationData(record, station);
        applyStationCountry(id.mid(9, 2).toUpper(), station);
    } else if (id.startsWith(QLatin1String("vrfi:")) && id.size() >= 7 && id.size() <= 9) {
        const auto record = KnowledgeDb::stationForVRStationCode(KnowledgeDb::VRStationCode(id.mid(5)));
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

    if (dt.timeSpec() == Qt::TimeZone) {
        return dt;
    }

    const auto tz = KnowledgeDb::timezoneForLocation(station.geo().latitude(), station.geo().longitude(), station.address().addressCountry());
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

Event ExtractorPostprocessorPrivate::processEvent(Event event) const
{
    event.setName(event.name().trimmed());

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

ProgramMembership ExtractorPostprocessorPrivate::processProgramMembership(ProgramMembership program) const
{
    program.setProgramName(program.programName().simplified());
     // avoid emitting spurious empty ProgramMembership objects caused by empty elements in JSON-LD/Microdata input
    if (program.programName().isEmpty() && !program.programName().isNull()) {
        program.setProgramName(QString());
    }
    program.setMember(processPerson(program.member()));
    return program;
}

template <typename T>
T ExtractorPostprocessorPrivate::processReservation(T res) const
{
    res.setUnderName(processPerson(res.underName().template value<Person>()));
    res.setPotentialAction(processActions(res.potentialAction()));
    res.setReservationNumber(res.reservationNumber().trimmed());
    res.setProgramMembershipUsed(processProgramMembership(res.programMembershipUsed()));
    return res;
}


Person ExtractorPostprocessorPrivate::processPerson(Person person) const
{
    person.setName(person.name().simplified());
    person.setFamilyName(person.familyName().simplified());
    person.setGivenName(person.givenName().simplified());

    // fill name with name parts, if it's empty
    if ((person.name().isEmpty() || person.name() == person.familyName() || person.name() == person.givenName())
        && !person.familyName().isEmpty() && !person.givenName().isEmpty())
    {
        person.setName(person.givenName() + QLatin1Char(' ') + person.familyName());
    }

    // strip prefixes, they break comparisons
    static const char* const honorificPrefixes[] = { "MR ", "MS ", "MRS " };
    for (auto prefix : honorificPrefixes) {
        if (person.name().startsWith(QLatin1String(prefix), Qt::CaseInsensitive)) {
            person.setName(person.name().mid(strlen(prefix)));
            break;
        }
    }

    return person;
}

PostalAddress ExtractorPostprocessorPrivate::processAddress(PostalAddress addr, const QString &phoneNumber, const GeoCoordinates &geo)
{
    // convert to ISO 3166-1 alpha-2 country codes
    if (addr.addressCountry().size() > 2) {
#if HAVE_KI18N_LOCALE_DATA
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
#else
        const auto isoCode = KContacts::Address::countryToISO(addr.addressCountry()).toUpper();
        if (!isoCode.isEmpty()) {
            addr.setAddressCountry(isoCode);

        // try ISO 3166-1 alpha-3, we get that e.g. from Flixbus
        } else if (addr.addressCountry().size() == 3) {
            const auto c = KnowledgeDb::countryIdFromIso3166_1alpha3(KnowledgeDb::CountryId3(addr.addressCountry()));
            if (c.isValid()) {
                addr.setAddressCountry(c.toString());
            }
        }
#endif
    }

    // upper case country codes
    if (addr.addressCountry().size() == 2) {
        addr.setAddressCountry(addr.addressCountry().toUpper());
    }

    // normalize strings
    addr.setStreetAddress(addr.streetAddress().simplified());
    addr.setAddressLocality(addr.addressLocality().simplified());
    addr.setAddressRegion(addr.addressRegion().simplified());

#ifdef HAVE_PHONENUMBER
    // recover country from phone number, if we have that
    if (!phoneNumber.isEmpty() && addr.addressCountry().size() != 2) {
        const auto phoneStr = phoneNumber.toStdString();
        const auto util = i18n::phonenumbers::PhoneNumberUtil::GetInstance();
        i18n::phonenumbers::PhoneNumber number;
        if (util->ParseAndKeepRawInput(phoneStr, "ZZ", &number) == i18n::phonenumbers::PhoneNumberUtil::NO_PARSING_ERROR) {
            std::string isoCode;
            util->GetRegionCodeForNumber(number, &isoCode);
            if (!isoCode.empty()) {
                addr.setAddressCountry(QString::fromStdString(isoCode));
            }
        }
    }
#endif

    if (geo.isValid() && addr.addressCountry().isEmpty()) {
        addr.setAddressCountry(KnowledgeDb::countryForCoordinate(geo.latitude(), geo.longitude()));
    }

    addr = ExtractorUtil::extractPostalCode(addr);
    return addr;
}

QString ExtractorPostprocessorPrivate::processPhoneNumber(const QString &phoneNumber, const PostalAddress &addr)
{
#ifdef HAVE_PHONENUMBER
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
    return phoneNumber;
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
    if (!dt.isValid() || dt.timeSpec() == Qt::TimeZone) {
        return dt;
    }

    const auto tz = KnowledgeDb::timezoneForLocation(place.geo().latitude(), place.geo().longitude(), place.address().addressCountry());
    if (!tz.isValid()) {
        return dt;
    }

    // prefer our timezone over externally provided UTC offset, if they match
    if (dt.timeSpec() == Qt::OffsetFromUTC && tz.offsetFromUtc(dt) != dt.offsetFromUtc()) {
        qCDebug(Log) << "UTC offset clashes with expected timezone!" << dt << dt.offsetFromUtc() << tz.id() << tz.offsetFromUtc(dt);
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
