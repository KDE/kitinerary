/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "iatabcbpparser.h"
#include "logging.h"
#include "reservationconverter.h"
#include "stringutil.h"

#include "iata/iatabcbp.h"

#include "knowledgedb/airportdb.h"
#include "knowledgedb/trainstationdb.h"

#include <KItinerary/Flight>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/Organization>
#include <KItinerary/Person>
#include <KItinerary/Place>
#include <KItinerary/Reservation>
#include <KItinerary/Ticket>
#include <KItinerary/TrainTrip>

#include <QJsonObject>
#include <QVariant>

using namespace Qt::Literals::StringLiterals;
using namespace KItinerary;

QList<QVariant> IataBcbpParser::parse(const QString &message, const QDateTime &externalIssueDate) {
    IataBcbp bcbp(message);
    if (!bcbp.isValid()) {
        return {};
    }
    return parse(bcbp, externalIssueDate.isValid() ? externalIssueDate : QDateTime({1970, 1, 1}, {}));
}

[[nodiscard]] static QString ticketNumber(QStringView airlineNumericCode, QStringView documentNumber)
{
    if (airlineNumericCode.size() != 3 || documentNumber.size() != 10) {
        return {};
    }

    if (std::any_of(airlineNumericCode.begin(), airlineNumericCode.end(), [](QChar c) { return !c.isDigit(); })
     || std::any_of(documentNumber.begin(), documentNumber.end(), [](QChar c) { return !c.isDigit(); })) {
        return {};
    }

    if (std::all_of(documentNumber.begin(), documentNumber.end(), [](QChar c) { return c == '0'_L1; })) {
        return {};
    }

    return airlineNumericCode + ' '_L1 + documentNumber;
}

// check if one of the airports is actually a train station, and convert the reservation accordingly
[[nodiscard]] QVariant checkModeOfTransport(FlightReservation &&res)
{
    const auto flight = res.reservationFor().value<Flight>();
    const auto from = KnowledgeDb::IataCode(flight.departureAirport().iataCode());
    const auto to = KnowledgeDb::IataCode(flight.arrivalAirport().iataCode());

    if (KnowledgeDb::coordinateForAirport(from).isValid() && KnowledgeDb::coordinateForAirport(to).isValid()) {
        return res;
    }
    if (KnowledgeDb::stationForIataCode(from).coordinate.isValid() || KnowledgeDb::stationForIataCode(to).coordinate.isValid()) {
        auto trainRes = JsonLdDocument::fromJsonSingular(ReservationConverter::flightToTrain(JsonLdDocument::toJson(res))).value<TrainReservation>();
        auto trip = trainRes.reservationFor().value<TrainTrip>();
        {
            auto station = trip.departureStation();
            station.setName(from.toString());
            trip.setDepartureStation(station);
        }
        {
            auto station = trip.arrivalStation();
            station.setName(to.toString());
            trip.setArrivalStation(station);
        }
        trainRes.setReservationFor(std::move(trip));
        return trainRes;
    }
    return res;
}

QList<QVariant> IataBcbpParser::parse(const IataBcbp &bcbp, const QDateTime &contextDate) {
    const auto count = bcbp.uniqueMandatorySection().numberOfLegs();
    const auto issueDate = bcbp.uniqueConditionalSection().dateOfIssue(contextDate);

    QList<QVariant> result;
    result.reserve(count);

    Person person;
    {
        const auto fullName = bcbp.uniqueMandatorySection().passengerName();
        const auto idx = fullName.indexOf('/'_L1);
        if (idx > 0 && idx < fullName.size() - 1) {
            person.setFamilyName(fullName.left(idx));
            person.setGivenName(fullName.mid(idx + 1));
        } else {
            person.setName(fullName);
        }
    }


    for (auto i = 0; i < count; ++i) {
        Flight flight;

        const auto rms = bcbp.repeatedMandatorySection(i);
        flight.setDepartureDay(rms.dateOfFlight(issueDate.isValid() ? QDateTime(issueDate, {}) : contextDate));

        Airport dep;
        dep.setIataCode(rms.fromCityAirportCode());
        flight.setDepartureAirport(dep);
        Airport arr;
        arr.setIataCode(rms.toCityAirportCode());
        flight.setArrivalAirport(arr);
        Airline airline;
        airline.setIataCode(rms.operatingCarrierDesignator());
        flight.setAirline(airline);
        flight.setFlightNumber(StringUtil::stripLeadingZeros(rms.flightNumber()));

        FlightReservation res;
        res.setReservationFor(flight);
        res.setPassengerSequenceNumber(StringUtil::stripLeadingZeros(rms.checkinSequenceNumber()));
        res.setAirplaneSeat(StringUtil::stripLeadingZeros(rms.seatNumber()));
        res.setReservationNumber(rms.operatingCarrierPNRCode());
        res.setUnderName(person);

        Ticket ticket;
        ticket.setTicketToken("aztecCode:"_L1 + bcbp.rawData());
        const auto rcs = bcbp.repeatedConditionalSection(i);
        ticket.setTicketNumber(ticketNumber(rcs.airlineNumericCode(), rcs.documentNumber()));
        res.setReservedTicket(ticket);

        if (!rcs.frequenFlyerNumber().isEmpty()) {
            ProgramMembership program;
            program.setMembershipNumber(rcs.frequenFlyerNumber());
            program.setProgramName(rcs.frequentFlyerAirlineDesignator());
            program.setMember(person);
            res.setProgramMembershipUsed(program);
        }

        result.push_back(checkModeOfTransport(std::move(res)));
    }

    return result;
}
