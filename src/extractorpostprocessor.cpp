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
#include "calendarhandler.h"
#include "jsonlddocument.h"
#include "airportdb/airportdb.h"
#include "iatabcbpparser.h"

#include <KItinerary/BusTrip>
#include <KItinerary/Flight>
#include <KItinerary/Place>
#include <KItinerary/Reservation>
#include <KItinerary/Ticket>
#include <KItinerary/TrainTrip>

#include <QDebug>
#include <QTimeZone>

#include <algorithm>

using namespace KItinerary;
namespace KItinerary {
class ExtractorPostprocessorPrivate
{
public:
    template <typename ObjT, typename PropT>
    ObjT processProperty(ObjT obj, const char *name, PropT (ExtractorPostprocessorPrivate::*processor)(PropT) const) const;

    QVariant processFlightReservation(FlightReservation res) const;
    Flight processFlight(Flight flight) const;
    QVariant processAirport(QVariant airport) const;
    QVariant processAirline(QVariant airline) const;
    void processFlightTime(Flight &flight, const char *timePropName, const char *airportPropName) const;
    QVariant processReservation(QVariant res) const;

    bool filterReservation(const QVariant &res) const;
    bool filterLodgingReservation(const LodgingReservation &res) const;
    bool filterFlight(const Flight &flight) const;
    bool filterAirport(const Airport &airport) const;
    bool filterTrainOrBusTrip(const QVariant &trip) const;
    bool filterTrainOrBusStation(const QVariant &station) const;

    QVector<QVariant> m_data;
    QDateTime m_contextDate;
};
}

ExtractorPostprocessor::ExtractorPostprocessor()
    : d(new ExtractorPostprocessorPrivate)
{
}

ExtractorPostprocessor::ExtractorPostprocessor(ExtractorPostprocessor &&) = default;
ExtractorPostprocessor::~ExtractorPostprocessor() = default;

void ExtractorPostprocessor::process(const QVector<QVariant> &data)
{
    d->m_data.reserve(data.size());
    for (auto elem : data) {
        if (elem.userType() == qMetaTypeId<FlightReservation>()) {
            elem = d->processFlightReservation(elem.value<FlightReservation>());
        } else if (elem.userType() == qMetaTypeId<TrainReservation>()) {
            elem = d->processReservation(elem);
        } else if (elem.userType() == qMetaTypeId<LodgingReservation>()) {
            elem = d->processReservation(elem);
        } else if (elem.userType() == qMetaTypeId<BusReservation>()) {
            elem = d->processReservation(elem);
        }

        if (d->filterReservation(elem)) {
            d->m_data.push_back(elem);
        }
    }

    std::stable_sort(d->m_data.begin(), d->m_data.end(), [](const QVariant &lhs, const QVariant &rhs) {
        return CalendarHandler::startDateTime(lhs) < CalendarHandler::startDateTime(rhs);
    });
}

QVector<QVariant> ExtractorPostprocessor::result() const
{
    return d->m_data;
}

void ExtractorPostprocessor::setContextDate(const QDateTime& dt)
{
    d->m_contextDate = dt;
}

template <typename ObjT, typename PropT>
ObjT ExtractorPostprocessorPrivate::processProperty(ObjT obj, const char *name, PropT (ExtractorPostprocessorPrivate::*processor)(PropT) const) const
{
    auto value = JsonLdDocument::readProperty(obj, name).template value<PropT>();
    value = (this->*processor)(value);
    JsonLdDocument::writeProperty(obj, name, value);
    return obj;
}

QVariant ExtractorPostprocessorPrivate::processFlightReservation(FlightReservation res) const
{
    // expand ticketToken for IATA BCBP data
    auto bcbp = res.reservedTicket().value<Ticket>().ticketToken();
    if (!bcbp.isEmpty()) {
        if (bcbp.startsWith(QLatin1String("aztecCode:"))) {
            bcbp = bcbp.mid(10);
        } else if (bcbp.startsWith(QLatin1String("qrCode:"))) {
            bcbp = bcbp.mid(7);
        }
        const auto bcbpData = IataBcbpParser::parse(bcbp, m_contextDate.date());
        if (bcbpData.size() == 1) {
            res = JsonLdDocument::apply(bcbpData.at(0), res).value<FlightReservation>();
        }
    }

    res = processReservation(res).value<FlightReservation>();
    res = processProperty(res, "reservationFor", &ExtractorPostprocessorPrivate::processFlight);
    return res;
}

Flight ExtractorPostprocessorPrivate::processFlight(Flight flight) const
{
    flight = processProperty(flight, "departureAirport", &ExtractorPostprocessorPrivate::processAirport);
    flight = processProperty(flight, "arrivalAirport", &ExtractorPostprocessorPrivate::processAirport);
    flight = processProperty(flight, "airline", &ExtractorPostprocessorPrivate::processAirline);

    processFlightTime(flight, "boardingTime", "departureAirport");
    processFlightTime(flight, "departureTime", "departureAirport");
    processFlightTime(flight, "arrivalTime", "arrivalAirport");

    return flight;
}

QVariant ExtractorPostprocessorPrivate::processAirport(QVariant airport) const
{
    // clean up name
    const auto name = JsonLdDocument::readProperty(airport, "name").toString();
    JsonLdDocument::writeProperty(airport, "name", name.trimmed());

    // complete missing IATA codes
    auto iataCode = JsonLdDocument::readProperty(airport, "iataCode").toString();
    if (iataCode.isEmpty()) {
        iataCode = AirportDb::iataCodeFromName(name).toString();
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

QVariant ExtractorPostprocessorPrivate::processAirline(QVariant airline) const
{
    const auto name = JsonLdDocument::readProperty(airline, "name").toString();
    JsonLdDocument::writeProperty(airline, "name", name.trimmed());
    return airline;
}

void ExtractorPostprocessorPrivate::processFlightTime(Flight &flight, const char *timePropName, const char *airportPropName) const
{
    const auto airport = JsonLdDocument::readProperty(flight, airportPropName);
    const auto iataCode = JsonLdDocument::readProperty(airport, "iataCode").toString();
    if (iataCode.isEmpty()) {
        return;
    }

    auto dt = JsonLdDocument::readProperty(flight, timePropName).toDateTime();
    if (!dt.isValid()) {
        return;
    }

    if (dt.date().year() <= 1970 && flight.departureDay().isValid()) { // we just have the time, but not the day
        dt.setDate(flight.departureDay());
        JsonLdDocument::writeProperty(flight, timePropName, dt);
    }

    if (dt.timeSpec() == Qt::TimeZone) {
        return;
    }

    const auto tz = AirportDb::timezoneForAirport(AirportDb::IataCode{iataCode});
    if (!tz.isValid()) {
        return;
    }

    // prefer our timezone over externally provided UTC offset, if they match
    if (dt.timeSpec() == Qt::OffsetFromUTC && tz.offsetFromUtc(dt) != dt.offsetFromUtc()) {
        return;
    }

    if (dt.timeSpec() == Qt::OffsetFromUTC || dt.timeSpec() == Qt::LocalTime) {
        dt.setTimeSpec(Qt::TimeZone);
        dt.setTimeZone(tz);
    } else if (dt.timeSpec() == Qt::UTC) {
        dt = dt.toTimeZone(tz);
    }
    // if we updated from UTC offset to timezone spec here, QDateTime will compare equal
    // and the auto-generated property code will not actually update the property
    // so, clear the property first to force an update
    JsonLdDocument::writeProperty(flight, timePropName, QDateTime());
    JsonLdDocument::writeProperty(flight, timePropName, dt);
}

QVariant ExtractorPostprocessorPrivate::processReservation(QVariant res) const
{
    const auto viewUrl = JsonLdDocument::readProperty(res, "url").toUrl();
    const auto modUrl = JsonLdDocument::readProperty(res, "modifyReservationUrl").toUrl();
    const auto cancelUrl = JsonLdDocument::readProperty(res, "cancelReservationUrl").toUrl();
    // remove duplicated urls
    if (modUrl.isValid() && viewUrl == modUrl) {
        JsonLdDocument::removeProperty(res, "modifyReservationUrl");
    }
    if (cancelUrl.isValid() && viewUrl == cancelUrl) {
        JsonLdDocument::removeProperty(res, "cancelReservationUrl");
    }

    return res;
}

bool ExtractorPostprocessorPrivate::filterReservation(const QVariant &res) const
{
    const auto resFor = JsonLdDocument::readProperty(res, "reservationFor");
    if (resFor.isNull()) {
        return false;
    }

    if (resFor.userType() == qMetaTypeId<Flight>()) {
        return filterFlight(resFor.value<Flight>());
    } else if (resFor.userType() == qMetaTypeId<TrainTrip>()) {
        return filterTrainOrBusTrip(resFor);
    } else if (resFor.userType() == qMetaTypeId<BusTrip>()) {
        return filterTrainOrBusTrip(resFor);
    }

    if (res.userType() == qMetaTypeId<LodgingReservation>()) {
        return filterLodgingReservation(res.value<LodgingReservation>());
    }
    return true;
}

bool ExtractorPostprocessorPrivate::filterLodgingReservation(const LodgingReservation &res) const
{
    return res.checkinTime().isValid() && res.checkoutTime().isValid();
}

bool ExtractorPostprocessorPrivate::filterFlight(const Flight &flight) const
{
    // this will be valid if either boarding time, departure time or departure day is set
    const auto validDate = flight.departureDay().isValid();
    return filterAirport(flight.departureAirport())
           && filterAirport(flight.arrivalAirport())
           && validDate;
}

bool ExtractorPostprocessorPrivate::filterAirport(const Airport &airport) const
{
    return !airport.iataCode().isEmpty() || !airport.name().isEmpty();
}

bool ExtractorPostprocessorPrivate::filterTrainOrBusTrip(const QVariant &trip) const
{
    const auto depDt = JsonLdDocument::readProperty(trip, "departureTime").toDateTime();
    const auto arrDt = JsonLdDocument::readProperty(trip, "arrivalTime").toDateTime();
    return filterTrainOrBusStation(JsonLdDocument::readProperty(trip, "departureStation"))
           && filterTrainOrBusStation(JsonLdDocument::readProperty(trip, "arrivalStation"))
           && depDt.isValid() && arrDt.isValid();
}

bool ExtractorPostprocessorPrivate::filterTrainOrBusStation(const QVariant &station) const
{
    return !JsonLdDocument::readProperty(station, "name").toString().isEmpty();
}
