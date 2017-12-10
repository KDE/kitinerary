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

#include "extractorpostprocessor.h"
#include "datatypes.h"
#include "jsonlddocument.h"
#include "airportdb/airportdb.h"

#include <QDebug>
#include <QTimeZone>

void ExtractorPostprocessor::process(const QVector<QVariant> &data)
{
    m_data.reserve(data.size());
    for (const auto &d : data) {
        if (d.userType() == qMetaTypeId<FlightReservation>()) {
            m_data.push_back(processFlightReservation(d));
        } else {
            m_data.push_back(d);
        }
    }
}

QVector<QVariant> ExtractorPostprocessor::result() const
{
    return m_data;
}

QVariant ExtractorPostprocessor::processProperty(QVariant obj, const char *name, QVariant (ExtractorPostprocessor::*processor)(QVariant) const) const
{
    auto value = JsonLdDocument::readProperty(obj, name);
    value = (this->*processor)(value);
    JsonLdDocument::writeProperty(obj, name, value);
    return obj;
}

QVariant ExtractorPostprocessor::processFlightReservation(QVariant res) const
{
    return processProperty(res, "reservationFor", &ExtractorPostprocessor::processFlight);
}

QVariant ExtractorPostprocessor::processFlight(QVariant flight) const
{
    flight = processProperty(flight, "departureAirport", &ExtractorPostprocessor::processAirport);
    flight = processProperty(flight, "arrivalAirport", &ExtractorPostprocessor::processAirport);

    processFlightTime(flight, "departureTime", "departureAirport");
    processFlightTime(flight, "arrivalTime", "arrivalAirport");

    return flight;
}

QVariant ExtractorPostprocessor::processAirport(QVariant airport) const
{
    // complete missing IATA codes
    auto iataCode = JsonLdDocument::readProperty(airport, "iataCode").toString();
    if (iataCode.isEmpty()) {
        iataCode = AirportDb::iataCodeFromName(JsonLdDocument::readProperty(airport, "name").toString()).toString();
        if (!iataCode.isEmpty()) {
            JsonLdDocument::writeProperty(airport, "iataCode", iataCode);
        }
    }

    // complete missing geo coordinates
    auto geo = JsonLdDocument::readProperty(airport, "geo");
    if (!geo.value<GeoCoordinates>().isValid()) {
        const auto coord = AirportDb::coordinateForAirport(AirportDb::IataCode{iataCode});
        if (coord.isValid()) {
            geo = QVariant::fromValue(GeoCoordinates());
            JsonLdDocument::writeProperty(geo, "latitude", coord.latitude);
            JsonLdDocument::writeProperty(geo, "longitude", coord.longitude);
            JsonLdDocument::writeProperty(airport, "geo", geo);
        }
    }

    return airport;
}

void ExtractorPostprocessor::processFlightTime(QVariant &flight, const char *timePropName, const char *airportPropName) const
{
    const auto airport = JsonLdDocument::readProperty(flight, airportPropName);
    const auto iataCode = JsonLdDocument::readProperty(airport, "iataCode").toString();
    if (iataCode.isEmpty()) {
        return;
    }

    auto dt = JsonLdDocument::readProperty(flight, timePropName).toDateTime();
    if (!dt.isValid() || dt.timeSpec() != Qt::LocalTime) {
        return;
    }

    const auto tz = AirportDb::timezoneForAirport(AirportDb::IataCode{iataCode});
    if (!tz.isValid()) {
        return;
    }

    dt.setTimeZone(tz);
    JsonLdDocument::writeProperty(flight, timePropName, dt);
}
