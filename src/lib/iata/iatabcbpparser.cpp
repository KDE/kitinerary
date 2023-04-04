/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "iatabcbpparser.h"
#include "logging.h"
#include "iata/iatabcbp.h"

#include <KItinerary/Flight>
#include <KItinerary/Organization>
#include <KItinerary/Person>
#include <KItinerary/Place>
#include <KItinerary/Reservation>
#include <KItinerary/Ticket>

#include <QVariant>

using namespace KItinerary;

static QString stripLeadingZeros(const QString &s)
{
    const auto it = std::find_if(s.begin(), s.end(), [](const QChar &c) { return c != QLatin1Char('0'); });
    const auto d = std::distance(s.begin(), it);
    return s.mid(d);
}

QList<QVariant> IataBcbpParser::parse(const QString &message,
                                      const QDateTime &externalIssueDate) {
    IataBcbp bcbp(message);
    if (!bcbp.isValid()) {
        return {};
    }
    return parse(bcbp, externalIssueDate.isValid() ? externalIssueDate : QDateTime({1970, 1, 1}, {}));
}

QList<QVariant> IataBcbpParser::parse(const IataBcbp &bcbp,
                                      const QDateTime &contextDate) {
    const auto count = bcbp.uniqueMandatorySection().numberOfLegs();
    const auto issueDate = bcbp.uniqueConditionalSection().dateOfIssue(contextDate);

    QList<QVariant> result;
    result.reserve(count);

    Person person;
    {
        const auto fullName = bcbp.uniqueMandatorySection().passengerName();
        const auto idx = fullName.indexOf(QLatin1Char('/'));
        if (idx > 0 && idx < fullName.size() - 1) {
            person.setFamilyName(fullName.left(idx));
            person.setGivenName(fullName.mid(idx + 1));
        } else {
            person.setName(fullName);
        }
    }

    Ticket ticket;
    ticket.setTicketToken(QStringLiteral("aztecCode:") + bcbp.rawData());

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
        flight.setFlightNumber(stripLeadingZeros(rms.flightNumber()));

        FlightReservation res;
        res.setReservationFor(flight);
        res.setPassengerSequenceNumber(stripLeadingZeros(rms.checkinSequenceNumber()));
        res.setAirplaneSeat(stripLeadingZeros(rms.seatNumber()));
        res.setReservationNumber(rms.operatingCarrierPNRCode());
        res.setUnderName(person);
        res.setReservedTicket(ticket);

        const auto rcs = bcbp.repeatedConditionalSection(i);
        if (!rcs.frequenFlyerNumber().isEmpty()) {
            ProgramMembership program;
            program.setMembershipNumber(rcs.frequenFlyerNumber());
            program.setProgramName(rcs.frequentFlyerAirlineDesignator());
            program.setMember(person);
            res.setProgramMembershipUsed(program);
        }

        result.push_back(std::move(res));
    }

    return result;
}
