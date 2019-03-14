/*
   Copyright (c) 2017-2019 Volker Krause <vkrause@kde.org>

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

#include "flightpostprocessor_p.h"
#include "extractorpostprocessor_p.h"
#include "extractorutil.h"

#include "knowledgedb/airportdb.h"

#include <KItinerary/Flight>
#include <KItinerary/Organization>

#include <QDateTime>
#include <QTimeZone>

using namespace KItinerary;

Flight FlightPostProcessor::processFlight(Flight flight) const
{
    flight.setDepartureAirport(processAirport(flight.departureAirport()));
    flight.setArrivalAirport(processAirport(flight.arrivalAirport()));
    flight.setAirline(processAirline(flight.airline()));
    flight.setBoardingTime(processFlightTime(flight.boardingTime(), flight, flight.departureAirport()));
    flight.setDepartureTime(processFlightTime(flight.departureTime(), flight, flight.departureAirport()));
    flight.setArrivalTime(processFlightTime(flight.arrivalTime(), flight, flight.arrivalAirport()));
    flight = ExtractorUtil::extractTerminals(flight);
    return flight;
}

Airport FlightPostProcessor::processAirport(Airport airport) const
{
    // clean up name
    airport.setName(airport.name().simplified());

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

    return ExtractorPostprocessorPrivate::processPlace(airport);
}

Airline FlightPostProcessor::processAirline(Airline airline) const
{
    airline.setName(airline.name().trimmed());
    return airline;
}

QDateTime FlightPostProcessor::processFlightTime(QDateTime dt, const Flight &flight, const Airport &airport) const
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
