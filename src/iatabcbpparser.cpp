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
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "iatabcbpparser.h"
#include "logging.h"

#include <KItinerary/Flight>
#include <KItinerary/Reservation>
#include <KItinerary/Person>
#include <KItinerary/Place>

#include <QDate>
#include <QVariant>

using namespace KItinerary;

namespace KItinerary {
enum Constants {
    UniqueMandatorySize = 23,
    RepeastedMandatorySize = 37,
    FormatCode = 'M',
    ElectronicTicketIndicator = 'E',
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

static int parseRepeatedMandatorySection(const QStringRef& msg, const QDate &issueDate, FlightReservation& res)
{
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
    if (issueDate.isValid()) {
        const auto days = msg.mid(21, 3).toInt() - 1;
        QDate date(issueDate.year(), 1, 1);
        date = date.addDays(days);
        if (date >= issueDate) {
            flight.setDepartureDay(date);
        } else {
            flight.setDepartureDay(QDate(issueDate.year() + 1, 1, 1).addDays(days));
        }
    }
    res.setReservationFor(QVariant::fromValue(flight));

    // 1x Compartment code

    res.setAirplaneSeat(stripLeadingZeros(msg.mid(25, 4)).toString());

    // 5x Checkin sequence number
    // 1x Passenger status

    // field size of conditional section + airline use section
    return readHexValue(msg.mid(35), 2);
}
}

QVector<QVariant> IataBcbpParser::parse(const QString& message, const QDate &issueDate)
{
    if (message.size() < (UniqueMandatorySize + RepeastedMandatorySize)) {
        qCWarning(Log) << "IATA BCBP code too short";
        return {};
    }
    if (message.at(0) != QLatin1Char(FormatCode) || !message.at(1).isDigit() || message.at(22) != QLatin1Char(ElectronicTicketIndicator)) {
        qCWarning(Log) << "IATA BCBP code invalid unique mandatory section format";
        return {};
    }

    // parse unique mandatory section
    const auto legCount = message.at(1).toLatin1() - '0';
    QVector<QVariant> result;
    result.reserve(legCount);
    FlightReservation res1;

    Person person;
    const auto fullName = message.midRef(2, 20).trimmed();
    // TODO split in family and given name
    person.setName(fullName.toString());
    res1.setUnderName(QVariant::fromValue(person));

    const auto varSize = parseRepeatedMandatorySection(message.midRef(UniqueMandatorySize), issueDate, res1);
    int index = UniqueMandatorySize + RepeastedMandatorySize;
    if (message.size() < (index + varSize)) {
        qCWarning(Log) << "IATA BCBP code too short for conditional section in first leg" << varSize << message.size();
        return {};
    }

    if (varSize > 0) {
        // parse unique conditional section
        if (message.at(index) != QLatin1Char(BeginOfVersionNumber)) {
            qCWarning(Log) << "IATA BCBP unique conditional section has invalid format";
            return {};
        }
        // 1x version number
        // 2x field size
        // baggage tags, information about boarding pass source, not really interesting for us

        // skip repeated conditional section, containing mainly bonus program data
        // skip for airline use section
        index += varSize;
    }

    result.push_back(QVariant::fromValue(res1));

    // all following legs only contain repeated sections, copy content from the unique ones from the first leg
    for (int i = 1; i < legCount; ++i) {
        if (message.size() < (index + RepeastedMandatorySize)) {
            qCWarning(Log) << "IATA BCBP repeated mandatory section too short" << i;
            return {};
        }

        FlightReservation res = res1;
        const auto varSize = parseRepeatedMandatorySection(message.midRef(index), issueDate, res);
        index += RepeastedMandatorySize;
        if (message.size() < (index + varSize)) {
            qCWarning(Log) << "IATA BCBP repeated conditional section too short" << i;
            return {};
        }

        // skip repeated conditional section
        // skip for airline use section

        index += varSize;
        result.push_back(QVariant::fromValue(res));
    }

    // optional security section at the end, not interesting for us

    return result;
}
