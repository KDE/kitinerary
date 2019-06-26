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
#include "locationutil.h"

#include "knowledgedb/airportdb.h"

#include <KItinerary/Flight>
#include <KItinerary/Organization>
#include <KItinerary/Place>

#include <QDateTime>
#include <QDebug>
#include <QTimeZone>

using namespace KItinerary;

enum {
    AirplaneSpeedLowerBound = 250, // km/h, turboprop aircraft, and a bit lower than average cruise speed to account for takeoff/landing
    AirplaneSpeedUpperBound = 2140, // km/h, Concorde, so a bit excessive
};

Flight FlightPostProcessor::processFlight(Flight flight)
{
    lookupAirportCodes(flight.departureAirport(), m_departureCodes);
    lookupAirportCodes(flight.arrivalAirport(), m_arrivalCodes);

    // if we have an ambiguous airport on one end, see if we can pick based on the travel time
    const auto duration = flight.departureTime().secsTo(flight.arrivalTime());
    pickAirportByDistance(duration, m_departureCodes, m_arrivalCodes);
    pickAirportByDistance(duration, m_arrivalCodes, m_departureCodes);

    flight.setDepartureAirport(processAirport(flight.departureAirport(), m_departureCodes));
    flight.setArrivalAirport(processAirport(flight.arrivalAirport(), m_arrivalCodes));
    flight.setAirline(processAirline(flight.airline()));
    flight.setBoardingTime(processFlightTime(flight.boardingTime(), flight, m_departureCodes));
    flight.setDepartureTime(processFlightTime(flight.departureTime(), flight, m_departureCodes));
    flight.setArrivalTime(processFlightTime(flight.arrivalTime(), flight, m_arrivalCodes));
    flight = ExtractorUtil::extractTerminals(flight);
    return flight;
}

Airport FlightPostProcessor::processAirport(Airport airport, const std::vector<KnowledgeDb::IataCode> &codes) const
{
    // complete missing IATA codes
    if (airport.iataCode().isEmpty() && codes.size() == 1) {
        airport.setIataCode(codes[0].toString());
    }

    // complete missing geo coordinates
    auto geo = airport.geo();
    if (!geo.isValid() && codes.size() == 1) {
        const auto coord = KnowledgeDb::coordinateForAirport(codes[0]);
        if (coord.isValid()) {
            geo.setLatitude(coord.latitude);
            geo.setLongitude(coord.longitude);
            airport.setGeo(geo);
        }
    }

    // add country, if all candidates are from the same country
    auto addr = airport.address();
    if (addr.addressCountry().isEmpty() && codes.size() >= 1) {
        const auto isoCode = KnowledgeDb::countryForAirport(codes[0]);
        if (isoCode.isValid() && std::all_of(codes.begin(), codes.end(), [isoCode](const auto iataCode) { return KnowledgeDb::countryForAirport(iataCode) == isoCode; })) {
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

QDateTime FlightPostProcessor::processFlightTime(QDateTime dt, const Flight &flight, const std::vector<KnowledgeDb::IataCode> &codes) const
{
    if (!dt.isValid()) {
        return dt;
    }

    if (dt.date().year() <= 1970 && flight.departureDay().isValid()) { // we just have the time, but not the day
        dt.setDate(flight.departureDay());
    }

    if (dt.timeSpec() == Qt::TimeZone || codes.empty()) {
        return dt;
    }

    const auto tz = KnowledgeDb::timezoneForAirport(KnowledgeDb::IataCode{codes[0]});
    if (!tz.isValid() || !std::all_of(codes.begin(), codes.end(), [tz](const auto &iataCode) { return KnowledgeDb::timezoneForAirport(iataCode) == tz; })) {
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

void FlightPostProcessor::lookupAirportCodes(const Airport &airport, std::vector<KnowledgeDb::IataCode>& codes) const
{
    if (!airport.iataCode().isEmpty()) {
        codes.push_back(KnowledgeDb::IataCode(airport.iataCode()));
        return;
    }

    // TODO if we don't need this elsewhere, maybe merge those two methods and do this logic internally more efficently?
    const auto code = KnowledgeDb::iataCodeFromName(airport.name());
    if (code.isValid()) {
        codes.push_back(code);
    } else {
        codes = KnowledgeDb::iataCodesFromName(airport.name());
    }
}

void FlightPostProcessor::pickAirportByDistance(int duration, const std::vector<KnowledgeDb::IataCode>& startCodes, std::vector<KnowledgeDb::IataCode>& codes) const
{
    if (duration <= 0 || startCodes.empty() || codes.size() <= 1) {
        return;
    }

    // ensure we have coordinates for all start points
    if (!std::all_of(startCodes.begin(), startCodes.end(), [](const auto code) { return KnowledgeDb::coordinateForAirport(code).isValid(); })) {
        return;
    }

    int lowerBoundDistance = AirplaneSpeedLowerBound * (duration / 3.6);
    if (duration < 3600) { // for ultra short flights the takeoff/landing overhead is so big that our expected speed range doesn't work reliable anymore
        lowerBoundDistance /= 2;
    }
    const int upperBoundDistance = AirplaneSpeedUpperBound * (duration / 3.6);

    for (auto it = codes.begin(); it != codes.end();) {
        const auto destCoord = KnowledgeDb::coordinateForAirport(*it);
        if (!destCoord.isValid()) {
            continue;
        }

        bool outOfRange = true;
        for (const auto startCode : startCodes) {
            const auto startCoord =  KnowledgeDb::coordinateForAirport(startCode);
            const auto dist = LocationUtil::distance({startCoord.latitude, startCoord.longitude}, {destCoord.latitude, destCoord.longitude});
            outOfRange = outOfRange && (dist > upperBoundDistance || dist < lowerBoundDistance);
        }
        if (outOfRange) {
            it = codes.erase(it);
        } else {
            ++it;
        }
    }
}
