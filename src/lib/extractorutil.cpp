/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "extractorutil.h"
#include "text/terminalfinder_p.h"

#include <KItinerary/Flight>
#include <KItinerary/Place>

#include <QDebug>
#include <QRegularExpression>

using namespace KItinerary;

static QString trimAirportName(QStringView in)
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
    static TerminalFinder finder(u"^.*", u"$");
    const auto result = finder.find(name);
    if (result.hasResult()) {
        const auto airportName = trimAirportName(QStringView(name).left(result.start));

        // try to recurse, sometimes this is indeed repeated...
        QString recName;
        QString recTerminal;
        std::tie(recName, recTerminal) = splitAirportName(airportName);
        if (recName == name || recTerminal.isEmpty()) {
            return std::make_tuple(airportName, result.name);
        } else {
            return std::make_tuple(recName, recTerminal);
        }
    }

    return std::make_tuple(name, QString());
}

Flight ExtractorUtil::extractTerminals(Flight flight)
{
    if (flight.departureTerminal().isEmpty()) {
        auto a = flight.departureAirport();
        QString name;
        QString terminal;
        std::tie(name, terminal) = splitAirportName(a.name());
        a.setName(name);
        flight.setDepartureAirport(a);
        flight.setDepartureTerminal(terminal);
    }

    if (flight.arrivalTerminal().isEmpty()) {
        auto a = flight.arrivalAirport();
        QString name;
        QString terminal;
        std::tie(name, terminal) = splitAirportName(a.name());
        a.setName(name);
        flight.setArrivalAirport(a);
        flight.setArrivalTerminal(terminal);
    }

    return flight;
}
