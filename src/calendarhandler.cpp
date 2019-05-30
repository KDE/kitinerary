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
#include "calendarhandler.h"
#include "jsonlddocument.h"
#include "logging.h"
#include "mergeutil.h"
#include "sortutil.h"

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

#ifdef HAVE_KCAL
#include <KCalCore/Alarm>
#include <KCalCore/Calendar>
#include <KCalCore/Event>
#endif

#include <KContacts/Address>

#include <KLocalizedString>

#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>

using namespace KItinerary;

static QString formatAddress(const PostalAddress &addr)
{
    KContacts::Address a;
    a.setStreet(addr.streetAddress());
    a.setPostalCode(addr.postalCode());
    a.setLocality(addr.addressLocality());
    a.setCountry(KContacts::Address::ISOtoCountry(addr.addressCountry()));
    return a.formattedAddress();
}

static QString formatAddressSingleLine(const PostalAddress &addr)
{
    return formatAddress(addr).replace(QLatin1String("\n\n"), QLatin1String("\n")).replace(QLatin1Char('\n'), QLatin1String(", "));
}

#ifdef HAVE_KCAL
using namespace KCalCore;
static void fillFlightReservation(const QVector<QVariant> &reservations, const KCalCore::Event::Ptr &event);
static void fillTrainReservation(const TrainReservation &reservation, const KCalCore::Event::Ptr &event);
static void fillBusReservation(const BusReservation &reservation, const KCalCore::Event::Ptr &event);
static void fillLodgingReservation(const LodgingReservation &reservation, const KCalCore::Event::Ptr &event);
static void fillEventReservation(const QVector<QVariant> &reservations, const KCalCore::Event::Ptr &event);
static void fillGeoPosition(const QVariant &place, const KCalCore::Event::Ptr &event);
static void fillFoodReservation(const FoodEstablishmentReservation &reservation, const KCalCore::Event::Ptr &event);
static void fillRentalCarReservation(const RentalCarReservation &reservation, const KCalCore::Event::Ptr &event);
static void fillTaxiReservation(const TaxiReservation &reservation, const KCalCore::Event::Ptr &event);
#endif

QSharedPointer<KCalCore::Event> CalendarHandler::findEvent(const QSharedPointer<KCalCore::Calendar> &calendar, const QVariant &reservation)
{
#ifdef HAVE_KCAL
    if (!JsonLd::canConvert<Reservation>(reservation)) {
        return {};
    }

    const auto dt = SortUtil::startDateTime(reservation).date();
    const auto events = calendar->events(dt);
    for (const auto &event : events) {
        if (!event->uid().startsWith(QLatin1String("KIT-"))) {
            continue;
        }
        const auto otherRes = CalendarHandler::reservationsForEvent(event);
        for (const auto &other : otherRes) {
            if (MergeUtil::isSame(other, reservation)) {
                return event;
            }
        }
    }
#else
    Q_UNUSED(calendar);
    Q_UNUSED(reservation);
#endif

    return {};
}

QVector<QVariant> CalendarHandler::reservationsForEvent(const QSharedPointer<KCalCore::Event> &event)
{
#ifdef HAVE_KCAL
    const auto payload = event->customProperty("KITINERARY", "RESERVATION").toUtf8();
    const auto json = QJsonDocument::fromJson(payload).array();
    return JsonLdDocument::fromJson(json);
#else
    Q_UNUSED(event);
    return {};
#endif
}

void CalendarHandler::fillEvent(const QVector<QVariant> &reservations, const QSharedPointer<KCalCore::Event> &event)
{
    if (reservations.isEmpty()) {
        return;
    }

#ifdef HAVE_KCAL
    // TODO pass reservationS into all functions below for multi-traveler support
    const auto &reservation = reservations.at(0);
    const int typeId = reservation.userType();
    if (typeId == qMetaTypeId<FlightReservation>()) {
        fillFlightReservation(reservations, event);
    } else if (typeId == qMetaTypeId<LodgingReservation>()) {
        fillLodgingReservation(reservation.value<LodgingReservation>(), event);
    } else if (typeId == qMetaTypeId<TrainReservation>()) {
        fillTrainReservation(reservation.value<TrainReservation>(), event);
    } else if (typeId == qMetaTypeId<BusReservation>()) {
        fillBusReservation(reservation.value<BusReservation>(), event);
    } else if (JsonLd::isA<EventReservation>(reservation)) {
        fillEventReservation(reservations, event);
    } else if (JsonLd::isA<FoodEstablishmentReservation>(reservation)) {
        fillFoodReservation(reservation.value<FoodEstablishmentReservation>(), event);
    } else if (JsonLd::isA<RentalCarReservation>(reservation)) {
        fillRentalCarReservation(reservation.value<RentalCarReservation>(), event);
    } else if (JsonLd::isA<TaxiReservation>(reservation)) {
        fillTaxiReservation(reservation.value<TaxiReservation>(), event);
    } else {
        return;
    }

    if (!event->uid().startsWith(QLatin1String("KIT-"))) {
        event->setUid(QLatin1String("KIT-") + event->uid());
    }

    const auto payload = QJsonDocument(JsonLdDocument::toJson(reservations)).toJson(QJsonDocument::Compact);
    event->setCustomProperty("KITINERARY", "RESERVATION", QString::fromUtf8(payload));
#else
    Q_UNUSED(event);
#endif
}

#ifdef HAVE_KCAL
static QString airportDisplayCode(const Airport &airport)
{
    return airport.iataCode().isEmpty() ? airport.name() : airport.iataCode();
}

static void fillFlightReservation(const QVector<QVariant> &reservations, const KCalCore::Event::Ptr &event)
{
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
        desc.push_back(i18n("Departure gate: %1", departureGate));
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

static void fillTrainReservation(const TrainReservation &reservation, const KCalCore::Event::Ptr &event)
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

static void fillBusReservation(const BusReservation &reservation, const KCalCore::Event::Ptr &event)
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

static void fillLodgingReservation(const LodgingReservation &reservation, const KCalCore::Event::Ptr &event)
{
    const auto lodgingBusiness = reservation.reservationFor().value<LodgingBusiness>();

    event->setSummary(i18n("Hotel reservation: %1", lodgingBusiness.name()));
    event->setLocation(formatAddressSingleLine(lodgingBusiness.address()));
    fillGeoPosition(lodgingBusiness, event);

    event->setDtStart(QDateTime(reservation.checkinTime().date(), QTime()));
    event->setDtEnd(QDateTime(reservation.checkoutTime().date(), QTime()));
    event->setAllDay(true);
    event->setDescription(i18n("Check-in: %1\nCheck-out: %2\nBooking reference: %3",
                               QLocale().toString(reservation.checkinTime().time(), QLocale::ShortFormat),
                               QLocale().toString(reservation.checkoutTime().time(), QLocale::ShortFormat),
                               reservation.reservationNumber()));
    event->setTransparency(KCalCore::Event::Transparent);
}

static void fillEventReservation(const QVector<QVariant> &reservations, const KCalCore::Event::Ptr &event)
{
    const auto ev = reservations.at(0).value<EventReservation>().reservationFor().value<KItinerary::Event>();
    Place location;
    if (JsonLd::canConvert<Place>(ev.location())) {
        location = JsonLd::convert<Place>(ev.location());
    }

    event->setSummary(ev.name());
    event->setLocation(location.name());
    fillGeoPosition(location, event);
    event->setDtStart(ev.startDate());
    event->setDtEnd(ev.endDate());
    event->setAllDay(false);

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

    QStringList desc;
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

static void fillGeoPosition(const QVariant &place, const KCalCore::Event::Ptr &event)
{
    if (!JsonLd::canConvert<Place>(place)) {
        return;
    }
    const auto geo = JsonLd::convert<Place>(place).geo();
    if (!geo.isValid()) {
        return;
    }

    event->setHasGeo(true);
    event->setGeoLatitude(geo.latitude());
    event->setGeoLongitude(geo.longitude());
}

static void fillFoodReservation(const FoodEstablishmentReservation &reservation, const KCalCore::Event::Ptr &event)
{
    const auto foodEstablishment = reservation.reservationFor().value<FoodEstablishment>();

    event->setSummary(i18n("Restaurant reservation: %1", foodEstablishment.name()));
    event->setLocation(formatAddressSingleLine(foodEstablishment.address()));
    fillGeoPosition(foodEstablishment, event);

    event->setDtStart(reservation.startTime());
    auto endTime = reservation.endTime();
    if (!endTime.isValid()) {
        endTime = QDateTime(reservation.startTime().date(), QTime(23, 59, 59));
    }
    event->setDtEnd(endTime);
    event->setAllDay(false);

    QStringList desc;
    if (reservation.partySize() > 0) {
        desc.push_back(i18n("Number Of People: %1", reservation.partySize()));
    }
    if (!reservation.reservationNumber().isEmpty()) {
        desc.push_back(i18n("Reservation reference: %1", reservation.reservationNumber()));
    }
    const auto person = reservation.underName().value<KItinerary::Person>();
    if (!person.name().isEmpty()) {
        desc.push_back(i18n("Under name: %1", person.name()));
    }
    event->setDescription(desc.join(QLatin1Char('\n')));
}

static void fillRentalCarReservation(const RentalCarReservation &reservation, const KCalCore::Event::Ptr &event)
{
    const auto rentalCalPickup = reservation.pickupLocation();
    const auto addressPickUp = rentalCalPickup.address();
    const auto rentalCar = reservation.reservationFor().value<RentalCar>();
    event->setSummary(i18n("Rental Car reservation: %1", rentalCar.name()));
    event->setLocation(formatAddressSingleLine(addressPickUp));
    fillGeoPosition(rentalCalPickup, event);

    event->setDtStart(reservation.pickupTime());
    event->setDtEnd(reservation.dropoffTime());
    event->setAllDay(false);
    event->setTransparency(KCalCore::Event::Transparent);
    event->setSummary(i18n("Rent car reservation: %1", rentalCar.name()));

    const QString pickUpAddress = formatAddress(addressPickUp);
    const auto rentalCalDropOff = reservation.dropoffLocation();
    const auto addressDropOff = rentalCalDropOff.address();
    const QString dropAddress = formatAddress(addressDropOff);

    const QString description = i18n("Reservation reference: %1\nUnder name: %2\n\nPickUp location: %3\n\nDropoff Location: %4",
                                     reservation.reservationNumber(),
                                     reservation.underName().value<KItinerary::Person>().name(),
                                     pickUpAddress,
                                     dropAddress);

    event->setDescription(description);
}

static void fillTaxiReservation(const TaxiReservation &reservation, const KCalCore::Event::Ptr &event)
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
    event->setTransparency(KCalCore::Event::Transparent);
    //TODO event->setSummary(i18n("Rent car reservation: %1", rentalCar.name()));
    const QString pickUpAddress = formatAddress(addressPickUp);

    const QString description = i18n("Reservation reference: %1\nUnder name: %2\nPickUp location: %3",
                                     reservation.reservationNumber(),
                                     reservation.underName().value<KItinerary::Person>().name(),
                                     pickUpAddress);

    event->setDescription(description);
}

#endif
