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
#include "datatypes.h"
#include "jsonlddocument.h"
#include "semantic_debug.h"

#include <KCalCore/Alarm>

#include <KLocalizedString>

using namespace KCalCore;

void CalendarHandler::fillEvent(const QVariant &reservation, const KCalCore::Event::Ptr &event)
{
    const int typeId = reservation.userType();
    if (typeId == qMetaTypeId<FlightReservation>()) {
        return fillFlightReservation(reservation, event);
    } else if (typeId == qMetaTypeId<LodgingReservation>()) {
        return fillLodgingReservation(reservation, event);
    } else if (typeId == qMetaTypeId<TrainReservation>()) {
        return fillTrainReservation(reservation, event);
    }
}

void CalendarHandler::fillFlightReservation(const QVariant &reservation, const KCalCore::Event::Ptr &event)
{
    const auto flight = JsonLdDocument::readProperty(reservation, "reservationFor");
    const auto airline = JsonLdDocument::readProperty(flight, "airline");
    const auto depPort = JsonLdDocument::readProperty(flight, "departureAirport");
    const auto arrPort = JsonLdDocument::readProperty(flight, "arrivalAirport");
    if (flight.isNull() || airline.isNull() || depPort.isNull() || arrPort.isNull()) {
        qCDebug(SEMANTIC_LOG) << "got invalid flight reservation";
        return;
    }

    const auto flightNumber = JsonLdDocument::readProperty(airline, "iataCode").toString()
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
        if (departureGate.isEmpty())
            alarm->setDisplayAlarm(i18n("Boarding for flight %1", flightNumber));
        else
            alarm->setDisplayAlarm(i18n("Boarding for flight %1 at gate %2", flightNumber, departureGate));
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

void CalendarHandler::fillTrainReservation(const QVariant &reservation, const KCalCore::Event::Ptr &event)
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

void CalendarHandler::fillLodgingReservation(const QVariant &reservation, const KCalCore::Event::Ptr &event)
{
    const auto lodgingBusiness = JsonLdDocument::readProperty(reservation, "reservationFor");
    const auto address = JsonLdDocument::readProperty(lodgingBusiness, "address");
    if (lodgingBusiness.isNull() || address.isNull()) {
        return;
    }

    event->setSummary(i18n("Hotel reservation: %1",
                           JsonLdDocument::readProperty(lodgingBusiness, "name").toString()
                           ));
    event->setLocation(i18n("%1, %2 %3, %4",
                            JsonLdDocument::readProperty(address, "streetAddress").toString(),
                            JsonLdDocument::readProperty(address, "postalCode").toString(),
                            JsonLdDocument::readProperty(address, "addressLocality").toString(),
                            JsonLdDocument::readProperty(address, "addressCountry").toString()
                            ));
    fillGeoPosition(lodgingBusiness, event);
    event->setDtStart(QDateTime(JsonLdDocument::readProperty(reservation, "checkinDate").toDate(), QTime()));
    event->setDtEnd(QDateTime(JsonLdDocument::readProperty(reservation, "checkoutDate").toDate(), QTime()));
    event->setAllDay(true);
    event->setDescription(i18n("Booking reference: %1",
                               JsonLdDocument::readProperty(reservation, "reservationNumber").toString()
                               ));
    event->setTransparency(Event::Transparent);
}

void CalendarHandler::fillGeoPosition(const QVariant &place, const KCalCore::Event::Ptr &event)
{
    const auto geo = JsonLdDocument::readProperty(place, "geo");
    if (geo.isNull()) {
        return;
    }

    event->setHasGeo(true);
    event->setGeoLatitude(JsonLdDocument::readProperty(geo, "latitude").toFloat());
    event->setGeoLongitude(JsonLdDocument::readProperty(geo, "longitude").toFloat());
}
