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

#include "calendarhandler.h"
#include "jsonlddocument.h"
#include "logging.h"

#include <datatypes/place.h>
#include <datatypes/reservation.h>

#include <KCalCore/Alarm>

#include <KLocalizedString>

using namespace KCalCore;
using namespace KItinerary;

static void fillFlightReservation(const QVariant &reservation, const KCalCore::Event::Ptr &event);
static void fillTripReservation(const QVariant &reservation, const KCalCore::Event::Ptr &event);
static void fillTrainReservation(const QVariant &reservation, const KCalCore::Event::Ptr &event);
static void fillBusReservation(const QVariant &reservation, const KCalCore::Event::Ptr &event);
static void fillLodgingReservation(const LodgingReservation &reservation, const KCalCore::Event::Ptr &event);
static void fillGeoPosition(const QVariant &place, const KCalCore::Event::Ptr &event);

QDateTime CalendarHandler::startDateTime(const QVariant &reservation)
{
    if (reservation.userType() == qMetaTypeId<FlightReservation>()
        || reservation.userType() == qMetaTypeId<TrainReservation>()
        || reservation.userType() == qMetaTypeId<BusReservation>()) {
        const auto trip = JsonLdDocument::readProperty(reservation, "reservationFor");
        return JsonLdDocument::readProperty(trip, "departureTime").toDateTime();
    } else if (reservation.userType() == qMetaTypeId<LodgingReservation>()) {
        return reservation.value<LodgingReservation>().checkinTime();
    }
    return {};
}

Event::Ptr CalendarHandler::findEvent(const Calendar::Ptr &calendar, const QVariant &reservation)
{
    const auto bookingRef = JsonLdDocument::readProperty(reservation, "reservationNumber").toString();
    if (bookingRef.isEmpty()) {
        return {};
    }

    auto dt = startDateTime(reservation);
    if (reservation.userType() == qMetaTypeId<LodgingReservation>()) {
        dt = QDateTime(dt.date(), QTime());
    }

    const auto events = calendar->events(dt);
    for (const auto &event : events) {
        if (event->dtStart() == dt && event->uid().startsWith(bookingRef)) {
            return event;
        }
    }
    return {};
}

void CalendarHandler::fillEvent(const QVariant &reservation, const KCalCore::Event::Ptr &event)
{
    const int typeId = reservation.userType();
    if (typeId == qMetaTypeId<FlightReservation>()) {
        fillFlightReservation(reservation, event);
    } else if (typeId == qMetaTypeId<LodgingReservation>()) {
        fillLodgingReservation(reservation.value<LodgingReservation>(), event);
    } else if (typeId == qMetaTypeId<TrainReservation>()) {
        fillTrainReservation(reservation, event);
    } else if (typeId == qMetaTypeId<BusReservation>()) {
        fillBusReservation(reservation, event);
    }

    const auto bookingRef = JsonLdDocument::readProperty(reservation, "reservationNumber").toString();
    if (!event->uid().startsWith(bookingRef)) {
        event->setUid(bookingRef + QLatin1Char('-') + event->uid());
    }
}

static void fillFlightReservation(const QVariant &reservation, const KCalCore::Event::Ptr &event)
{
    const auto flight = JsonLdDocument::readProperty(reservation, "reservationFor");
    const auto airline = JsonLdDocument::readProperty(flight, "airline");
    const auto depPort = JsonLdDocument::readProperty(flight, "departureAirport");
    const auto arrPort = JsonLdDocument::readProperty(flight, "arrivalAirport");
    if (flight.isNull() || airline.isNull() || depPort.isNull() || arrPort.isNull()) {
        qCDebug(Log) << "got invalid flight reservation";
        return;
    }

    const QString flightNumber = JsonLdDocument::readProperty(airline, "iataCode").toString()
                              + QLatin1Char(' ')
                              + JsonLdDocument::readProperty(flight, "flightNumber").toString();

    event->setSummary(i18n("Flight %1 from %2 to %3", flightNumber,
                           JsonLdDocument::readProperty(depPort, "iataCode").toString(),
                           JsonLdDocument::readProperty(arrPort, "iataCode").toString()
                           ));
    event->setLocation(JsonLdDocument::readProperty(depPort, "name").toString());
    fillGeoPosition(depPort, event);
    event->setDtStart(JsonLdDocument::readProperty(flight, "departureTime").toDateTime());
    event->setDtEnd(JsonLdDocument::readProperty(flight, "arrivalTime").toDateTime());
    event->setAllDay(false);

    const auto boardingTime = JsonLdDocument::readProperty(flight, "boardingTime").toDateTime();
    const auto departureGate = JsonLdDocument::readProperty(flight, "departureGate").toString();
    if (boardingTime.isValid()) {
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

    QStringList desc;
    if (boardingTime.isValid()) {
        desc.push_back(i18n("Boarding time: %1", QLocale().toString(boardingTime.time(), QLocale::ShortFormat)));
    }
    if (!departureGate.isEmpty()) {
        desc.push_back(i18n("Departure gate: %1", departureGate));
    }
    auto s = JsonLdDocument::readProperty(reservation, "boardingGroup").toString();
    if (!s.isEmpty()) {
        desc.push_back(i18n("Boarding group: %1", s));
    }
    s = JsonLdDocument::readProperty(reservation, "airplaneSeat").toString();
    if (!s.isEmpty()) {
        desc.push_back(i18n("Seat: %1", s));
    }
    s = JsonLdDocument::readProperty(reservation, "reservationNumber").toString();
    if (!s.isEmpty()) {
        desc.push_back(i18n("Booking reference: %1", s));
    }
    event->setDescription(desc.join(QLatin1Char('\n')));
}

static void fillTripReservation(const QVariant &reservation, const KCalCore::Event::Ptr &event)
{
    const auto trip = JsonLdDocument::readProperty(reservation, "reservationFor");
    const auto depStation = JsonLdDocument::readProperty(trip, "departureStation");
    const auto arrStation = JsonLdDocument::readProperty(trip, "arrivalStation");

    event->setLocation(JsonLdDocument::readProperty(depStation, "name").toString());
    fillGeoPosition(depStation, event);
    event->setDtStart(JsonLdDocument::readProperty(trip, "departureTime").toDateTime());
    event->setDtEnd(JsonLdDocument::readProperty(trip, "arrivalTime").toDateTime());
    event->setAllDay(false);

    QStringList desc;
    auto s = JsonLdDocument::readProperty(trip, "departurePlatform").toString();
    if (!s.isEmpty()) {
        desc.push_back(i18n("Departure platform: %1", s));
    }
    const auto ticket = JsonLdDocument::readProperty(reservation, "reservedTicket");
    const auto seat = JsonLdDocument::readProperty(ticket, "ticketedSeat");
    s = JsonLdDocument::readProperty(seat, "seatSection").toString();
    if (!s.isEmpty()) {
        desc.push_back(i18n("Coach: %1", s));
    }
    s = JsonLdDocument::readProperty(seat, "seatNumber").toString();
    if (!s.isEmpty()) {
        desc.push_back(i18n("Seat: %1", s));
    }
    s = JsonLdDocument::readProperty(trip, "arrivalPlatform").toString();
    if (!s.isEmpty()) {
        desc.push_back(i18n("Arrival platform: %1", s));
    }
    s = JsonLdDocument::readProperty(reservation, "reservationNumber").toString();
    if (!s.isEmpty()) {
        desc.push_back(i18n("Booking reference: %1", s));
    }
    event->setDescription(desc.join(QLatin1Char('\n')));
}

static void fillTrainReservation(const QVariant &reservation, const KCalCore::Event::Ptr &event)
{
    const auto trip = JsonLdDocument::readProperty(reservation, "reservationFor");
    const auto depStation = JsonLdDocument::readProperty(trip, "departureStation");
    const auto arrStation = JsonLdDocument::readProperty(trip, "arrivalStation");
    if (trip.isNull() || depStation.isNull() || arrStation.isNull()) {
        return;
    }

    event->setSummary(i18n("Train %1 from %2 to %3",
                           JsonLdDocument::readProperty(trip, "trainNumber").toString(),
                           JsonLdDocument::readProperty(depStation, "name").toString(),
                           JsonLdDocument::readProperty(arrStation, "name").toString()
                           ));
    fillTripReservation(reservation, event);
}

static void fillBusReservation(const QVariant &reservation, const KCalCore::Event::Ptr &event)
{
    const auto trip = JsonLdDocument::readProperty(reservation, "reservationFor");
    const auto depStation = JsonLdDocument::readProperty(trip, "departureStation");
    const auto arrStation = JsonLdDocument::readProperty(trip, "arrivalStation");
    if (trip.isNull() || depStation.isNull() || arrStation.isNull()) {
        return;
    }

    event->setSummary(i18n("Bus %1 from %2 to %3",
                           JsonLdDocument::readProperty(trip, "busNumber").toString(),
                           JsonLdDocument::readProperty(depStation, "name").toString(),
                           JsonLdDocument::readProperty(arrStation, "name").toString()
                           ));
    fillTripReservation(reservation, event);
}

static void fillLodgingReservation(const LodgingReservation &reservation, const KCalCore::Event::Ptr &event)
{
    if (reservation.reservationFor().isNull()) {
        return;
    }
    const auto lodgingBusiness = reservation.reservationFor().value<LodgingBusiness>();
    const auto address = lodgingBusiness.address();

    event->setSummary(i18n("Hotel reservation: %1", lodgingBusiness.name()));
    event->setLocation(i18nc("<street>, <postal code> <city>, <country>", "%1, %2 %3, %4",
                             address.streetAddress(), address.postalCode(),
                             address.addressLocality(), address.addressCountry()));
    fillGeoPosition(lodgingBusiness, event);

    event->setDtStart(QDateTime(reservation.checkinTime().date(), QTime()));
    event->setDtEnd(QDateTime(reservation.checkoutTime().date(), QTime()));
    event->setAllDay(true);
    event->setDescription(i18n("Check-in: %1\nCheck-out: %2\nBooking reference: %3",
                               QLocale().toString(reservation.checkinTime().time(), QLocale::ShortFormat),
                               QLocale().toString(reservation.checkoutTime().time(), QLocale::ShortFormat),
                               reservation.reservationNumber()));
    event->setTransparency(Event::Transparent);
}

static void fillGeoPosition(const QVariant &place, const KCalCore::Event::Ptr &event)
{
    const auto geo = JsonLdDocument::readProperty(place, "geo").value<GeoCoordinates>();
    if (!geo.isValid()) {
        return;
    }

    event->setHasGeo(true);
    event->setGeoLatitude(geo.latitude());
    event->setGeoLongitude(geo.longitude());
}
