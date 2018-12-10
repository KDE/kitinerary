/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "iatabcbpparser.h"
#include "logging.h"

#include <KItinerary/Flight>
#include <KItinerary/Organization>
#include <KItinerary/Person>
#include <KItinerary/Place>
#include <KItinerary/Reservation>
#include <KItinerary/Ticket>

#include <QDate>
#include <QVariant>

using namespace KItinerary;

namespace KItinerary {
enum Constants {
    UniqueMandatorySize = 23,
    RepeatedMandatorySize = 37,
    FormatCode = 'M',
    BeginOfVersionNumber = '>'
};

static QStringRef stripLeadingZeros(const QStringRef &s)
{
    const auto it = std::find_if(s.begin(), s.end(), [](const QChar &c) { return c != QLatin1Char('0'); });
    const auto d = std::distance(s.begin(), it);
    return s.mid(d);
}

static int readHexValue(const QStringRef &s, int width)
{
    return s.mid(0, width).toInt(nullptr, 16);
}

static int parseRepeatedMandatorySection(const QStringRef& msg, FlightReservation& res)
{
    if (msg.size() < 24) { // pre-checking data, technically incomplete, but we can make use of this nevertheless
        qCWarning(Log) << "IATA BCBP repeated mandatory section too short";
        return -1; // error
    }
    res.setReservationNumber(msg.mid(0, 7).trimmed().toString());

    Flight flight;
    Airport airport;
    airport.setIataCode(msg.mid(7, 3).toString());
    flight.setDepartureAirport(airport);
    airport.setIataCode(msg.mid(10, 3).toString());
    flight.setArrivalAirport(airport);

    Airline airline;
    airline.setIataCode(msg.mid(13, 3).trimmed().toString());
    flight.setAirline(airline);
    flight.setFlightNumber(stripLeadingZeros(msg.mid(16, 5).trimmed()).toString());

    // 3x Date of flight, as days since Jan 1st
    // we don't know the year here, so use 1970, will be filled up or discarded by the caller
    const auto days = msg.mid(21, 3).toInt() - 1;
    flight.setDepartureDay(QDate(1970, 1, 1).addDays(days));
    res.setReservationFor(flight);

    if (msg.size() < RepeatedMandatorySize) {
        return 0;
    }

    // 1x Compartment code

    res.setAirplaneSeat(stripLeadingZeros(msg.mid(25, 4)).trimmed().toString());
    res.setPassengerSequenceNumber(stripLeadingZeros(msg.mid(29, 5)).trimmed().toString());

    // 1x Passenger status

    // field size of conditional section + airline use section
    return readHexValue(msg.mid(35), 2);
}
}

QVector<QVariant> IataBcbpParser::parse(const QString& message, const QDate &externalIssueDate)
{
    if (!IataBcbpParser::maybeIataBcbp(message)) {
        qCWarning(Log) << "IATA BCBP code too short for unique mandatory section, or invalid mandatory section format";
        return {};
    }

    // parse unique mandatory section
    const auto legCount = message.at(1).toLatin1() - '0';
    QVector<QVariant> result;
    result.reserve(legCount);
    FlightReservation res1;

    {
        Person person;
        const auto fullName = message.midRef(2, 20).trimmed();

        const auto idx = fullName.indexOf(QLatin1Char('/'));
        if (idx > 0 && idx < fullName.size() - 1) {
            person.setFamilyName(fullName.left(idx).toString());
            person.setGivenName(fullName.mid(idx + 1).toString());
        } else {
            person.setName(fullName.toString());
        }
        res1.setUnderName(person);
    }

    {
        Ticket ticket;
        ticket.setTicketToken(QStringLiteral("aztecCode:") + message);
        res1.setReservedTicket(ticket);
    }

    const auto varSize = parseRepeatedMandatorySection(message.midRef(UniqueMandatorySize), res1);
    if (varSize < 0) { // parser error
        return {};
    }
    int index = UniqueMandatorySize + RepeatedMandatorySize;
    auto issueDate = externalIssueDate;
    if (varSize > 0) {
        if (message.size() < (index + varSize)) {
            qCWarning(Log) << "IATA BCBP code too short for conditional section in first leg" << varSize << message.size();
            return {};
        }

        // parse unique conditional section, if there is one, otherwise we skip all of this assuming "for airline use"
        if (message.at(index) == QLatin1Char(BeginOfVersionNumber)) {
            // 1x version number
            // 2x field size of unique conditional section
            const auto uniqCondSize = readHexValue(message.midRef(index + 2), 2);
            if (uniqCondSize + 4 > varSize) {
                qCWarning(Log) << "IATA BCBP unique conditional section has invalid size" << varSize << uniqCondSize;
                return {};
            }

            // 1x passenger description
            // 1x source of checking
            // 1x source of boarding pass issuance

            // 4x date of issue of boarding pass
            // this only contains the last digit of the year (sic), but we assume it to be in the past
            // so this still gives us a 10 year range of correctly determined times
            if (uniqCondSize >= 11 && externalIssueDate.isValid() && message.at(index + 7).isDigit() && message.at(index + 10).isDigit()) {
                const auto year = message.at(index + 7).toLatin1() - '0';
                const auto days = message.midRef(index + 8, 3).toInt() - 1;
                if (year < 0 || year > 9 || days < 0 || days > 365) {
                    qCWarning(Log) << "IATA BCBP invalid boarding pass issue date format" << message.midRef(index + 7, 4);
                    return {};
                }

                auto currentYear = externalIssueDate.year() - externalIssueDate.year() % 10 + year;
                if (currentYear > externalIssueDate.year()) {
                    currentYear -= 10;
                }
                issueDate = QDate(currentYear, 1, 1).addDays(days);
            }

            // 1x document type
            // 3x airline code of boarding pass issuer
            // 3x 13x baggage tag numbers

            // skip repeated conditional section, containing mainly bonus program data
        }

        // skip for airline use section
        index += varSize;
    }

    result.push_back(res1);

    // all following legs only contain repeated sections, copy content from the unique ones from the first leg
    for (int i = 1; i < legCount; ++i) {
        if (message.size() < (index + RepeatedMandatorySize)) {
            qCWarning(Log) << "IATA BCBP repeated mandatory section too short" << i;
            return {};
        }

        FlightReservation res = res1;
        const auto varSize = parseRepeatedMandatorySection(message.midRef(index), res);
        if (varSize < 0) { // parser error
            return {};
        }
        index += RepeatedMandatorySize;
        if (message.size() < (index + varSize)) {
            qCWarning(Log) << "IATA BCBP repeated conditional section too short" << i;
            return {};
        }

        // skip repeated conditional section
        // skip for airline use section

        index += varSize;
        result.push_back(res);
    }

    // optional security section at the end, not interesting for us

    // complete departure dates with the now (hopefully known issue date)
    for (auto it = result.begin(); it != result.end(); ++it) {
        auto res = (*it).value<FlightReservation>();
        auto flight = res.reservationFor().value<Flight>();

        if (issueDate.isValid()) {
            const auto days = flight.departureDay().dayOfYear() - 1;
            QDate date(issueDate.year(), 1, 1);
            date = date.addDays(days);
            if (date >= issueDate) {
                flight.setDepartureDay(date);
            } else {
                flight.setDepartureDay(QDate(issueDate.year() + 1, 1, 1).addDays(days));
            }
        } else {
            flight.setDepartureDay(QDate());
        }

        res.setReservationFor(flight);
        *it = res;
    }

    return result;
}

bool IataBcbpParser::maybeIataBcbp(const QString &message)
{
    if (message.size() < UniqueMandatorySize) {
        return false;
    }
    if (message.at(0) != QLatin1Char(FormatCode) || !message.at(1).isDigit()) {
        return false;
    }

    return true;
}
