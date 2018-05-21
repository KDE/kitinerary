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

#include <KItinerary/BusTrip>
#include <KItinerary/Flight>
#include <KItinerary/Organization>
#include <KItinerary/Place>
#include <KItinerary/Reservation>
#include <KItinerary/TrainTrip>

#ifdef HAVE_KCAL
#include <KCalCore/Alarm>
#include <KCalCore/Calendar>
#include <KCalCore/Event>
#endif

#ifdef HAVE_KCONTACTS
#include <KContacts/Address>
#endif

#include <KLocalizedString>

#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>

using namespace KItinerary;

#ifdef HAVE_KCAL
using namespace KCalCore;
static void fillFlightReservation(const FlightReservation &reservation, const KCalCore::Event::Ptr &event);
static void fillTripReservation(const QVariant &reservation, const KCalCore::Event::Ptr &event);
static void fillTrainReservation(const TrainReservation &reservation, const KCalCore::Event::Ptr &event);
static void fillBusReservation(const BusReservation &reservation, const KCalCore::Event::Ptr &event);
static void fillLodgingReservation(const LodgingReservation &reservation, const KCalCore::Event::Ptr &event);
static void fillGeoPosition(const QVariant &place, const KCalCore::Event::Ptr &event);
#endif

QDateTime CalendarHandler::startDateTime(const QVariant &reservation)
{
    if (JsonLd::isA<FlightReservation>(reservation)) {
        const auto flight = reservation.value<FlightReservation>().reservationFor().value<Flight>();
        if (flight.departureTime().isValid()) {
            return flight.departureTime();
        }
        return QDateTime(flight.departureDay(), QTime());
    }
    if (JsonLd::isA<TrainReservation>(reservation)) {
        return reservation.value<TrainReservation>().reservationFor().value<TrainTrip>().departureTime();
    }
    if (JsonLd::isA<BusReservation>(reservation)) {
        return reservation.value<BusReservation>().reservationFor().value<BusTrip>().departureTime();
    }
    if (JsonLd::isA<LodgingReservation>(reservation)) {
        return reservation.value<LodgingReservation>().checkinTime();
    }
    if (JsonLd::isA<FoodEstablishmentReservation>(reservation)) {
        return reservation.value<FoodEstablishmentReservation>().startTime();
    }
    return {};
}

QSharedPointer<KCalCore::Event> CalendarHandler::findEvent(const QSharedPointer<KCalCore::Calendar> &calendar, const QVariant &reservation)
{
#ifdef HAVE_KCAL
    if (!JsonLd::canConvert<Reservation>(reservation)) {
        return {};
    }
    auto bookingRef = JsonLd::convert<Reservation>(reservation).reservationNumber();
    if (bookingRef.isEmpty()) {
        return {};
    }
    bookingRef.prepend(QLatin1String("KIT-"));

    auto dt = startDateTime(reservation);
    if (reservation.userType() == qMetaTypeId<LodgingReservation>()) {
        dt = QDateTime(dt.date(), QTime());
    }

    const auto events = calendar->events(dt.date());
    for (const auto &event : events) {
        if (!event->uid().startsWith(bookingRef)) {
            continue;
        }
        const auto otherRes = CalendarHandler::reservationForEvent(event);
        if (MergeUtil::isSame(otherRes, reservation)) {
            return event;
        }
    }
#else
    Q_UNUSED(calendar);
    Q_UNUSED(reservation);
#endif

    return {};
}

QVariant CalendarHandler::reservationForEvent(const QSharedPointer<KCalCore::Event> &event)
{
#ifdef HAVE_KCAL
    const auto payload = event->customProperty("KITINERARY", "RESERVATION").toUtf8();
    const auto json = QJsonDocument::fromJson(payload).array();
    const auto data = JsonLdDocument::fromJson(json);
    if (data.size() != 1) {
        return {};
    }
    return data.at(0);
#else
    Q_UNUSED(event);
    return {};
#endif
}

void CalendarHandler::fillEvent(const QVariant &reservation, const QSharedPointer<KCalCore::Event> &event)
{
#ifdef HAVE_KCAL
    const int typeId = reservation.userType();
    if (typeId == qMetaTypeId<FlightReservation>()) {
        fillFlightReservation(reservation.value<FlightReservation>(), event);
    } else if (typeId == qMetaTypeId<LodgingReservation>()) {
        fillLodgingReservation(reservation.value<LodgingReservation>(), event);
    } else if (typeId == qMetaTypeId<TrainReservation>()) {
        fillTrainReservation(reservation.value<TrainReservation>(), event);
    } else if (typeId == qMetaTypeId<BusReservation>()) {
        fillBusReservation(reservation.value<BusReservation>(), event);
    } else {
        return;
    }

    const auto bookingRef = JsonLd::convert<Reservation>(reservation).reservationNumber();
    if (!event->uid().startsWith(QLatin1String("KIT-") + bookingRef)) {
        event->setUid(QLatin1String("KIT-") + bookingRef + QLatin1Char('-') + event->uid());
    }

    const auto payload = QJsonDocument(JsonLdDocument::toJson({reservation})).toJson(QJsonDocument::Compact);
    event->setCustomProperty("KITINERARY", "RESERVATION", QString::fromUtf8(payload));
#else
    Q_UNUSED(reservation);
    Q_UNUSED(event);
#endif
}

#ifdef HAVE_KCAL
static void fillFlightReservation(const FlightReservation &reservation, const KCalCore::Event::Ptr &event)
{
    const auto flight = reservation.reservationFor().value<Flight>();
    const auto airline = flight.airline();
    const auto depPort = flight.departureAirport();
    const auto arrPort = flight.arrivalAirport();

    const QString flightNumber = airline.iataCode() + QLatin1Char(' ') + flight.flightNumber();

    event->setSummary(i18n("Flight %1 from %2 to %3", flightNumber, depPort.iataCode(), arrPort.iataCode()));
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
    if (!reservation.boardingGroup().isEmpty()) {
        desc.push_back(i18n("Boarding group: %1", reservation.boardingGroup()));
    }
    if (!reservation.airplaneSeat().isEmpty()) {
        desc.push_back(i18n("Seat: %1", reservation.airplaneSeat()));
    }
    if (!reservation.reservationNumber().isEmpty()) {
        desc.push_back(i18n("Booking reference: %1", reservation.reservationNumber()));
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

static void fillTrainReservation(const TrainReservation &reservation, const KCalCore::Event::Ptr &event)
{
    const auto trip = reservation.reservationFor().value<TrainTrip>();
    const auto depStation = trip.departureStation();
    const auto arrStation = trip.arrivalStation();

    event->setSummary(i18n("Train %1 from %2 to %3", trip.trainNumber(), depStation.name(), arrStation.name()));
    fillTripReservation(reservation, event);
}

static void fillBusReservation(const BusReservation &reservation, const KCalCore::Event::Ptr &event)
{
    const auto trip = reservation.reservationFor().value<BusTrip>();
    const auto depStation = trip.departureStation();
    const auto arrStation = trip.arrivalStation();

    event->setSummary(i18n("Bus %1 from %2 to %3", trip.busNumber(), depStation.name(), arrStation.name()));
    fillTripReservation(reservation, event);
}

static void fillLodgingReservation(const LodgingReservation &reservation, const KCalCore::Event::Ptr &event)
{
    const auto lodgingBusiness = reservation.reservationFor().value<LodgingBusiness>();
    const auto address = lodgingBusiness.address();

    event->setSummary(i18n("Hotel reservation: %1", lodgingBusiness.name()));
#ifdef HAVE_KCONTACTS
    event->setLocation(i18nc("<street>, <postal code> <city>, <country>", "%1, %2 %3, %4",
                             address.streetAddress(), address.postalCode(),
                             address.addressLocality(), KContacts::Address::ISOtoCountry(address.addressCountry())));
#endif
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
#endif
