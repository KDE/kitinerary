/*
   SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "config-kitinerary.h"
#include "calendarhandler.h"
#include "jsonlddocument.h"
#include "locationutil_p.h"
#include "logging.h"
#include "mergeutil.h"
#include "sortutil.h"

#include <KItinerary/BoatTrip>
#include <KItinerary/BusTrip>
#include <KItinerary/Event>
#include <KItinerary/Flight>
#include <KItinerary/Organization>
#include <KItinerary/Person>
#include <KItinerary/Place>
#include <KItinerary/RentalCar>
#include <KItinerary/Reservation>
#include <KItinerary/Ticket>
#include <KItinerary/TrainTrip>
#include <KItinerary/Visit>

#include <KCalendarCore/Alarm>
#include <KCalendarCore/Calendar>
#include <KCalendarCore/Event>

#include <KContacts/Address>

#include <KLocalizedString>

#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>

using namespace KItinerary;

static QString formatAddress(const PostalAddress &addr)
{
    return LocationUtil::toAddress(addr).formatted(KContacts::AddressFormatStyle::MultiLineInternational);
}

static QString formatAddressSingleLine(const PostalAddress &addr)
{
    return LocationUtil::toAddress(addr).formatted(KContacts::AddressFormatStyle::SingleLineInternational);
}

using namespace KCalendarCore;
static void fillFlightReservation(const QList<QVariant> &reservations,
                                  const KCalendarCore::Event::Ptr &event);
static void fillTrainReservation(const TrainReservation &reservation, const KCalendarCore::Event::Ptr &event);
static void fillBusReservation(const BusReservation &reservation, const KCalendarCore::Event::Ptr &event);
static void fillBoatReservation(const BoatReservation &reservation, const KCalendarCore::Event::Ptr &event);
static void fillLodgingReservation(const QList<QVariant> &reservations,
                                   const KCalendarCore::Event::Ptr &event);
static void fillEvent(const KItinerary::Event &ev, const KCalendarCore::Event::Ptr &event);
static void fillEventReservation(const QList<QVariant> &reservations,
                                 const KCalendarCore::Event::Ptr &event);
static void fillGeoPosition(const QVariant &place, const KCalendarCore::Event::Ptr &event);
static void fillFoodReservation(const FoodEstablishmentReservation &reservation, const KCalendarCore::Event::Ptr &event);
static void fillRentalCarReservation(const RentalCarReservation &reservation, const KCalendarCore::Event::Ptr &event);
static void fillTaxiReservation(const TaxiReservation &reservation, const KCalendarCore::Event::Ptr &event);

QList<QSharedPointer<KCalendarCore::Event>> CalendarHandler::findEvents(
    const QSharedPointer<KCalendarCore::Calendar> &calendar,
    const QVariant &reservation) {
    return findEvents(calendar.data(), reservation);
}

QList<QSharedPointer<KCalendarCore::Event>>
CalendarHandler::findEvents(KCalendarCore::Calendar *calendar,
                            const QVariant &reservation) {
    if (!(JsonLd::canConvert<Reservation>(reservation) || JsonLd::canConvert<KItinerary::Event>(reservation)) || !calendar) {
        return {};
    }

    QList<KCalendarCore::Event::Ptr> events;
    QList<KCalendarCore::Event::Ptr> results;
    const auto startDt = SortUtil::startDateTime(reservation);
    const auto endDt = SortUtil::endDateTime(reservation);
    if (startDt.isValid() && endDt.isValid() && startDt == startDt.date().startOfDay(startDt.timeZone())
        && std::abs(endDt.secsTo(endDt.date().endOfDay(endDt.timeZone()))) <= 1) {
        // looks like an all day event, don't adjust for timezones in that case
        events = calendar->events(startDt.date());
    } else if (startDt.isValid()) {
        // we know the exact day to search at
        events = calendar->events(startDt.toTimeZone(calendar->timeZone()).date());
    } else if (JsonLd::canConvert<Reservation>(reservation)) {
        // for minimal cancellations, we need to search in a larger range
        const auto res = JsonLd::convert<Reservation>(reservation);
        if (!res.modifiedTime().isValid() || res.reservationStatus() != Reservation::ReservationCancelled) {
            return {};
        }
        const auto date = res.modifiedTime().toTimeZone(calendar->timeZone()).date();
        events = calendar->events(date, date.addDays(180));
    }

    for (const auto &event : events) {
      if (!event->uid().startsWith(QLatin1StringView("KIT-"))) {
        continue;
      }
        const auto otherRes = CalendarHandler::reservationsForEvent(event);
        for (const auto &other : otherRes) {
            if (MergeUtil::isSame(other, reservation)) {
                results.push_back(event);
            }
        }
    }

    return results;
}

QList<QVariant> CalendarHandler::reservationsForEvent(
    const QSharedPointer<KCalendarCore::Event> &event) {
    const auto payload = event->customProperty("KITINERARY", "RESERVATION").toUtf8();
    const auto json = QJsonDocument::fromJson(payload).array();
    return JsonLdDocument::fromJson(json);
}

bool CalendarHandler::canCreateEvent(const QVariant &reservation)
{
    if (JsonLd::isA<FlightReservation>(reservation)) {
        const auto f = reservation.value<FlightReservation>().reservationFor().value<Flight>();
        if (f.departureTime().isValid() && f.arrivalTime().isValid()) {
            return true;
        }
    }
    return SortUtil::startDateTime(reservation).isValid();
}

void CalendarHandler::fillEvent(
    const QList<QVariant> &reservations,
    const QSharedPointer<KCalendarCore::Event> &event) {
    if (reservations.isEmpty()) {
        return;
    }

    // TODO pass reservationS into all functions below for multi-traveler support
    const auto &reservation = reservations.at(0);
    const int typeId = reservation.userType();
    if (typeId == qMetaTypeId<FlightReservation>()) {
        fillFlightReservation(reservations, event);
    } else if (typeId == qMetaTypeId<LodgingReservation>()) {
        fillLodgingReservation(reservations, event);
    } else if (typeId == qMetaTypeId<TrainReservation>()) {
        fillTrainReservation(reservation.value<TrainReservation>(), event);
    } else if (typeId == qMetaTypeId<BusReservation>()) {
        fillBusReservation(reservation.value<BusReservation>(), event);
    } else if (typeId == qMetaTypeId<BoatReservation>()) {
        fillBoatReservation(reservation.value<BoatReservation>(), event);
    } else if (JsonLd::isA<EventReservation>(reservation)) {
        fillEventReservation(reservations, event);
    } else if  (JsonLd::isA<Event>(reservation)) {
        fillEvent(reservation.value<KItinerary::Event>(), event);
    } else if (JsonLd::isA<FoodEstablishmentReservation>(reservation)) {
        fillFoodReservation(reservation.value<FoodEstablishmentReservation>(), event);
    } else if (JsonLd::isA<RentalCarReservation>(reservation)) {
        fillRentalCarReservation(reservation.value<RentalCarReservation>(), event);
    } else if (JsonLd::isA<TaxiReservation>(reservation)) {
        fillTaxiReservation(reservation.value<TaxiReservation>(), event);
    } else {
        return;
    }

    if (JsonLd::canConvert<Reservation>(reservation) && JsonLd::convert<Reservation>(reservation).reservationStatus() == Reservation::ReservationCancelled) {
        event->setTransparency(KCalendarCore::Event::Transparent);
        event->setSummary(i18nc("canceled train/flight/loading reservation", "Canceled: %1", event->summary()));
        event->clearAlarms();
    }

    if (!event->uid().startsWith(QLatin1StringView("KIT-"))) {
      event->setUid(QLatin1StringView("KIT-") + event->uid());
    }

    const auto payload = QJsonDocument(JsonLdDocument::toJson(reservations)).toJson(QJsonDocument::Compact);
    event->setCustomProperty("KITINERARY", "RESERVATION", QString::fromUtf8(payload));
}

static QString airportDisplayCode(const Airport &airport)
{
    return airport.iataCode().isEmpty() ? airport.name() : airport.iataCode();
}

static void fillFlightReservation(const QList<QVariant> &reservations,
                                  const KCalendarCore::Event::Ptr &event) {
    const auto flight = reservations.at(0).value<FlightReservation>().reservationFor().value<Flight>();
    const auto airline = flight.airline();
    const auto depPort = flight.departureAirport();
    const auto arrPort = flight.arrivalAirport();

    const QString flightNumber = airline.iataCode() + QLatin1Char(' ') + flight.flightNumber();

    event->setSummary(i18n("Flight %1 from %2 to %3", flightNumber, airportDisplayCode(depPort), airportDisplayCode(arrPort)));
    event->setLocation(depPort.name());
    fillGeoPosition(depPort, event);
    event->setDtStart(flight.departureTime());
    event->setDtEnd(flight.arrivalTime());
    event->setAllDay(false);

    const auto boardingTime = flight.boardingTime();
    const auto departureGate = flight.departureGate();
    if (boardingTime.isValid()) {
        const auto startOffset = Duration(event->dtStart(), boardingTime);
        const auto existinAlarms = event->alarms();
        const auto it = std::find_if(existinAlarms.begin(), existinAlarms.end(), [startOffset](const Alarm::Ptr &other) {
            return other->startOffset() == startOffset;
        });
        if (it == existinAlarms.end()) {
            Alarm::Ptr alarm(new Alarm(event.data()));
            alarm->setStartOffset(Duration(event->dtStart(), boardingTime));
            if (departureGate.isEmpty()) {
                alarm->setDisplayAlarm(i18n("Boarding for flight %1", flightNumber));
            } else {
                alarm->setDisplayAlarm(i18n("Boarding for flight %1 at gate %2", flightNumber, departureGate));
            }
            alarm->setEnabled(true);
            event->addAlarm(alarm);
        }
    }

    QStringList desc;
    if (boardingTime.isValid()) {
        desc.push_back(i18n("Boarding time: %1", QLocale().toString(boardingTime.time(), QLocale::ShortFormat)));
    }
    if (!departureGate.isEmpty()) {
        desc.push_back(i18nc("flight departure gate", "Departure gate: %1", departureGate));
    }

    for (const auto &r : reservations) {
        const auto reservation = r.value<FlightReservation>();
        const auto person = reservation.underName().value<KItinerary::Person>();
        if (!person.name().isEmpty()) {
            desc.push_back(person.name());
        }
        if (!reservation.boardingGroup().isEmpty()) {
            desc.push_back(i18n("Boarding group: %1", reservation.boardingGroup()));
        }
        if (!reservation.airplaneSeat().isEmpty()) {
            desc.push_back(i18n("Seat: %1", reservation.airplaneSeat()));
        }
        if (!reservation.reservationNumber().isEmpty()) {
            desc.push_back(i18n("Booking reference: %1", reservation.reservationNumber()));
        }
    }
    event->setDescription(desc.join(QLatin1Char('\n')));
}

static void fillTrainReservation(const TrainReservation &reservation, const KCalendarCore::Event::Ptr &event)
{
    const auto trip = reservation.reservationFor().value<TrainTrip>();
    const auto depStation = trip.departureStation();
    const auto arrStation = trip.arrivalStation();

    event->setSummary(i18n("Train %1 from %2 to %3", trip.trainNumber(), depStation.name(), arrStation.name()));
    event->setLocation(depStation.name());
    fillGeoPosition(depStation, event);
    event->setDtStart(trip.departureTime());
    event->setDtEnd(trip.arrivalTime());
    event->setAllDay(false);

    QStringList desc;
    if (!trip.departurePlatform().isEmpty()) {
        desc.push_back(i18n("Departure platform: %1", trip.departurePlatform()));
    }
    const auto ticket = reservation.reservedTicket().value<Ticket>();
    const auto seat = ticket.ticketedSeat();
    if (!seat.seatSection().isEmpty()) {
        desc.push_back(i18n("Coach: %1", seat.seatSection()));
    }
    if (!seat.seatNumber().isEmpty()) {
        desc.push_back(i18n("Seat: %1", seat.seatNumber()));
    }
    if (!trip.arrivalPlatform().isEmpty()) {
        desc.push_back(i18n("Arrival platform: %1", trip.arrivalPlatform()));
    }
    if (!reservation.reservationNumber().isEmpty()) {
        desc.push_back(i18n("Booking reference: %1", reservation.reservationNumber()));
    }
    event->setDescription(desc.join(QLatin1Char('\n')));
}

static void fillBusReservation(const BusReservation &reservation, const KCalendarCore::Event::Ptr &event)
{
    const auto trip = reservation.reservationFor().value<BusTrip>();
    const auto depStation = trip.departureBusStop();
    const auto arrStation = trip.arrivalBusStop();

    event->setSummary(i18n("Bus %1 from %2 to %3", trip.busNumber(), depStation.name(), arrStation.name()));
    event->setLocation(depStation.name());
    fillGeoPosition(depStation, event);
    event->setDtStart(trip.departureTime());
    event->setDtEnd(trip.arrivalTime());
    event->setAllDay(false);

    QStringList desc;
    const auto ticket = reservation.reservedTicket().value<Ticket>();
    const auto seat = ticket.ticketedSeat();
    if (!seat.seatNumber().isEmpty()) {
        desc.push_back(i18n("Seat: %1", seat.seatNumber()));
    }
    if (!reservation.reservationNumber().isEmpty()) {
        desc.push_back(i18n("Booking reference: %1", reservation.reservationNumber()));
    }
    event->setDescription(desc.join(QLatin1Char('\n')));
}

static void fillBoatReservation(const KItinerary::BoatReservation &reservation, const KCalendarCore::Event::Ptr &event)
{
    const auto trip = reservation.reservationFor().value<BoatTrip>();
    const auto depTerminal = trip.departureBoatTerminal();
    const auto arrTerminal = trip.arrivalBoatTerminal();

    event->setSummary(i18n("Ferry from %1 to %2", depTerminal.name(), arrTerminal.name()));
    event->setLocation(depTerminal.name());
    fillGeoPosition(depTerminal, event);
    event->setDtStart(trip.departureTime());
    event->setDtEnd(trip.arrivalTime());
    event->setAllDay(false);

    QStringList desc;
    const auto ticket = reservation.reservedTicket().value<Ticket>();
    if (!reservation.reservationNumber().isEmpty()) {
        desc.push_back(i18n("Booking reference: %1", reservation.reservationNumber()));
    }
    if (!ticket.ticketNumber().isEmpty() && ticket.ticketNumber() != reservation.reservationNumber()) {
        desc.push_back(i18n("Ticket number: %1", ticket.ticketNumber()));
    }
    event->setDescription(desc.join(QLatin1Char('\n')));
}

static void fillLodgingReservation(const QList<QVariant> &reservations,
                                   const KCalendarCore::Event::Ptr &event) {
    const auto reservation = reservations.at(0).value<LodgingReservation>();
    const auto lodgingBusiness = reservation.reservationFor().value<LodgingBusiness>();

    event->setSummary(i18n("Hotel reservation: %1", lodgingBusiness.name()));
    event->setLocation(formatAddressSingleLine(lodgingBusiness.address()));
    fillGeoPosition(lodgingBusiness, event);

    event->setDtStart(QDateTime(reservation.checkinTime().date(), QTime()));
    event->setDtEnd(QDateTime(reservation.checkoutTime().date(), QTime()));
    event->setAllDay(true);
    event->setTransparency(KCalendarCore::Event::Transparent);

    QStringList desc;
    if (reservation.checkinTime().isValid()) {
        desc.push_back(i18n("Check-in: %1", QLocale().toString(reservation.checkinTime().time(), QLocale::ShortFormat)));
    }
    if (reservation.checkoutTime().isValid()) {
        desc.push_back(i18n("Check-out: %1", QLocale().toString(reservation.checkoutTime().time(), QLocale::ShortFormat)));
    }
    if (!lodgingBusiness.telephone().isEmpty()) {
        desc.push_back(i18n("Phone: %1", lodgingBusiness.telephone()));
    }
    if (!lodgingBusiness.email().isEmpty()) {
        desc.push_back(i18n("Email: %1", lodgingBusiness.email()));
    }
    if (!lodgingBusiness.url().isEmpty()) {
        desc.push_back(i18n("Website: %1", lodgingBusiness.url().toString()));
    }

    for (const auto &r : reservations) {
        const auto reservation = r.value<LodgingReservation>();
        const auto person = reservation.underName().value<KItinerary::Person>();
        if (!person.name().isEmpty()) {
            desc.push_back(person.name());
        }
        if (!reservation.reservationNumber().isEmpty()) {
            desc.push_back(i18n("Booking reference: %1", reservation.reservationNumber()));
        }
    }
    event->setDescription(desc.join(QLatin1Char('\n')));
}

static void fillEvent(const KItinerary::Event &ev, const KCalendarCore::Event::Ptr &event)
{
    Place location;
    if (JsonLd::canConvert<Place>(ev.location())) {
        location = JsonLd::convert<Place>(ev.location());
    }

    event->setSummary(ev.name());
    event->setLocation(location.name());
    fillGeoPosition(location, event);
    event->setDtStart(ev.startDate());
    if (ev.endDate().isValid()) {
        event->setDtEnd(ev.endDate());
        // QDate::endOfDay adds 999ms, while endDate has that truncated
        event->setAllDay(ev.startDate() == ev.startDate().date().startOfDay(ev.startDate().timeZone())
                     && std::abs(ev.endDate().secsTo(ev.endDate().date().endOfDay(ev.endDate().timeZone()))) <= 1);
        if (event->allDay()) {
            event->setDtStart(QDateTime(event->dtStart().date(), QTime()));
            event->setDtEnd(QDateTime(event->dtEnd().date(), QTime()));
        }
    } else {
        event->setDtEnd(ev.startDate().addSecs(3600));
        event->setAllDay(false);
    }

    if (ev.doorTime().isValid()) {
        const auto startOffset = Duration(event->dtStart(), ev.doorTime());
        const auto existinAlarms = event->alarms();
        const auto it = std::find_if(existinAlarms.begin(), existinAlarms.end(), [startOffset](const Alarm::Ptr &other) {
            return other->startOffset() == startOffset;
        });
        if (it == existinAlarms.end()) {
            Alarm::Ptr alarm(new Alarm(event.data()));
            alarm->setStartOffset(Duration(event->dtStart(), ev.doorTime()));
            alarm->setDisplayAlarm(i18n("Entrance for %1", ev.name()));
            alarm->setEnabled(true);
            event->addAlarm(alarm);
        }
    }

    event->setDescription(formatAddress(location.address()) + QLatin1Char('\n'));
}

static void fillEventReservation(const QList<QVariant> &reservations,
                                 const KCalendarCore::Event::Ptr &event) {
    const auto ev = reservations.at(0).value<EventReservation>().reservationFor().value<KItinerary::Event>();
    fillEvent(ev, event);

    QStringList desc = { event->description() };
    for (const auto &r : reservations) {
        const auto reservation = r.value<EventReservation>();
        const auto person = reservation.underName().value<KItinerary::Person>();
        if (!person.name().isEmpty()) {
            desc.push_back(person.name());
        }
        // TODO: add seat information if present
        if (!reservation.reservationNumber().isEmpty()) {
            desc.push_back(i18n("Booking reference: %1", reservation.reservationNumber()));
        }
    }
    event->setDescription(desc.join(QLatin1Char('\n')));
}

static void fillGeoPosition(const QVariant &place, const KCalendarCore::Event::Ptr &event)
{
    if (!JsonLd::canConvert<Place>(place)) {
        return;
    }
    const auto geo = JsonLd::convert<Place>(place).geo();
    if (!geo.isValid()) {
        return;
    }

    event->setGeoLatitude(geo.latitude());
    event->setGeoLongitude(geo.longitude());
}

static void fillFoodReservation(const FoodEstablishmentReservation &reservation, const KCalendarCore::Event::Ptr &event)
{
    const auto foodEstablishment = reservation.reservationFor().value<FoodEstablishment>();

    event->setSummary(i18n("Restaurant reservation: %1", foodEstablishment.name()));
    event->setLocation(formatAddressSingleLine(foodEstablishment.address()));
    fillGeoPosition(foodEstablishment, event);

    event->setDtStart(reservation.startTime());
    auto endTime = reservation.endTime();
    if (!endTime.isValid()) {
        endTime = reservation.startTime().addSecs(7200); // if we have no end time, let's assume 2h
    }
    event->setDtEnd(endTime);
    event->setAllDay(false);

    QStringList desc;
    if (reservation.partySize() > 0) {
        desc.push_back(i18n("Number of people: %1", reservation.partySize()));
    }
    if (!reservation.reservationNumber().isEmpty()) {
        desc.push_back(i18n("Booking reference: %1", reservation.reservationNumber()));
    }
    const auto person = reservation.underName().value<KItinerary::Person>();
    if (!person.name().isEmpty()) {
        desc.push_back(i18n("Under name: %1", person.name()));
    }
    event->setDescription(desc.join(QLatin1Char('\n')));
}

static void fillRentalCarReservation(const RentalCarReservation &reservation, const KCalendarCore::Event::Ptr &event)
{
    const auto rentalCalPickup = reservation.pickupLocation();
    const auto addressPickUp = rentalCalPickup.address();
    const auto rentalCar = reservation.reservationFor().value<RentalCar>();
    event->setSummary(i18n("Rental car reservation: %1", rentalCar.name()));
    event->setLocation(rentalCalPickup.name());
    fillGeoPosition(rentalCalPickup, event);

    event->setDtStart(reservation.pickupTime());
    event->setDtEnd(reservation.dropoffTime());
    event->setAllDay(false);
    event->setTransparency(KCalendarCore::Event::Transparent);

    QStringList desc;
    if (!addressPickUp.isEmpty()) {
        desc.push_back(i18n("Pickup location: %1\n%2\n", rentalCalPickup.name(), formatAddress(addressPickUp)));
    }
    const auto dropOff = reservation.dropoffLocation();
    if (!dropOff.name().isEmpty()) {
        desc.push_back(i18n("Dropoff location: %1\n%2\n", dropOff.name(), formatAddress(dropOff.address())));
    }
    if (!reservation.reservationNumber().isEmpty()) {
        desc.push_back(i18n("Booking reference: %1", reservation.reservationNumber()));
    }
    const auto person = reservation.underName().value<KItinerary::Person>();
    if (!person.name().isEmpty()) {
        desc.push_back(i18n("Under name: %1", person.name()));
    }

    event->setDescription(desc.join(QLatin1Char('\n')));
}

static void fillTaxiReservation(const TaxiReservation &reservation, const KCalendarCore::Event::Ptr &event)
{
    const auto taxiPickup = reservation.pickupLocation();
    const auto addressPickUp = taxiPickup.address();
    //TODO const auto rentalCar = reservation.reservationFor().value<RentalCar>();
    //TODO event->setSummary(i18n("Rental Car reservation: %1", rentalCar.name()));
    event->setLocation(formatAddressSingleLine(addressPickUp));
    fillGeoPosition(taxiPickup, event);

    event->setDtStart(reservation.pickupTime());
    //TODO event->setDtEnd(reservation.dropoffTime());
    event->setAllDay(false);
    event->setTransparency(KCalendarCore::Event::Transparent);
    //TODO event->setSummary(i18n("Rent car reservation: %1", rentalCar.name()));
    const QString pickUpAddress = formatAddress(addressPickUp);

    const QString description = i18n("Reservation reference: %1\nUnder name: %2\nPickup location: %3",
                                     reservation.reservationNumber(),
                                     reservation.underName().value<KItinerary::Person>().name(),
                                     pickUpAddress);

    event->setDescription(description);
}

