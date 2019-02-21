/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

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

#include "extractorutil.h"

#include <KItinerary/Flight>
#include <KItinerary/Place>

#include <QDebug>
#include <QRegularExpression>

using namespace KItinerary;

static QString trimAirportName(const QStringRef &in)
{
    QString out = in.toString();
    while (!out.isEmpty()) {
        const auto c = out.at(out.size() - 1);
        if (c.isSpace() || c == QLatin1Char('-') || c == QLatin1Char(',')) {
            out.chop(1);
        } else {
            break;
        }
    }

    return out;
}

static std::tuple<QString, QString> splitAirportName(const QString &name)
{
    static QRegularExpression patterns[] = {
        QRegularExpression(QStringLiteral("^(.*) \\((?:terminal|aerogare) (.*)\\)$"), QRegularExpression::CaseInsensitiveOption),
        QRegularExpression(QStringLiteral("^(.*) \\((.*) (?:terminal|aerogare)\\)$"), QRegularExpression::CaseInsensitiveOption),
        QRegularExpression(QStringLiteral("^(.*)[ -](?:terminal|aerogare) (.*)$"), QRegularExpression::CaseInsensitiveOption),
    };

    for (const auto &re : patterns) {
        const auto match = re.match(name);
        if (match.hasMatch()) {
            const auto name = trimAirportName(match.capturedRef(1));

            // try to recurse, sometimes this is indeed repeated...
            QString recName, recTerminal;
            std::tie(recName, recTerminal) = splitAirportName(name);
            if (recName == name || recTerminal.isEmpty()) {
                return std::make_tuple(trimAirportName(match.capturedRef(1)), match.captured(2));
            } else {
                return std::make_tuple(recName, recTerminal);
            }
            break;
        }
    }

    return std::make_tuple(name, QString());
}

Flight ExtractorUtil::extractTerminals(Flight flight)
{
    if (flight.departureTerminal().isEmpty()) {
        auto a = flight.departureAirport();
        QString name, terminal;
        std::tie(name, terminal) = splitAirportName(a.name());
        a.setName(name);
        flight.setDepartureAirport(a);
        flight.setDepartureTerminal(terminal);
    }

    if (flight.arrivalTerminal().isEmpty()) {
        auto a = flight.arrivalAirport();
        QString name, terminal;
        std::tie(name, terminal) = splitAirportName(a.name());
        a.setName(name);
        flight.setArrivalAirport(a);
        flight.setArrivalTerminal(terminal);
    }

    return flight;
}

PostalAddress ExtractorUtil::extractPostalCode(PostalAddress addr)
{
    if (!addr.postalCode().isEmpty() || addr.addressLocality().isEmpty()) {
        return addr;
    }

    // ### this so far only covers the typical European numerical prefix case, we probably want
    // something for alpha-numeric and suffix cases too, if necessary we can also make this
    // conditional on addr.addressCountry()
    static QRegularExpression patterns[] = {
        QRegularExpression(QStringLiteral("^(\\d{4,8}) (.*)$"), QRegularExpression::CaseInsensitiveOption),
    };

    for (const auto &re : patterns) {
        const auto match = re.match(addr.addressLocality());
        if (match.hasMatch()) {
            addr.setAddressLocality(match.captured(2));
            addr.setPostalCode(match.captured(1));
            break;
        }
    }

    return addr;
}
