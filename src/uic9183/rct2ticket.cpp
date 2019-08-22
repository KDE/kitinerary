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

#include "rct2ticket.h"
#include "logging.h"
#include "uic9183ticketlayout.h"

#include <QDateTime>
#include <QDebug>

#include <cstring>

using namespace KItinerary;

namespace KItinerary {

class Rct2TicketPrivate : public QSharedData
{
public:
    QDate firstDayOfValidity() const;
    QDateTime parseTime(const QString &dateStr, const QString &timeStr) const;

    Uic9183TicketLayout layout;
    QDateTime contextDt;
};

}

QDate Rct2TicketPrivate::firstDayOfValidity() const
{
    const auto f = layout.text(3, 1, 48, 1);
    const auto it = std::find_if(f.begin(), f.end(), [](QChar c) { return c.isDigit(); });
    if (it == f.end()) {
        return {};
    }

    const auto dtStr = f.midRef(std::distance(f.begin(), it));
    auto dt = QDate::fromString(dtStr.left(10).toString(), QStringLiteral("dd.MM.yyyy"));
    if (dt.isValid()) {
        return dt;
    }
    dt = QDate::fromString(dtStr.left(8).toString(), QStringLiteral("dd.MM.yy"));
    if (dt.isValid()) {
        if (dt.year() < 2000) {
            dt.setDate(dt.year() + 100, dt.month(), dt.day());
        }
        return dt;
    }
    dt = QDate::fromString(dtStr.left(4).toString(), QStringLiteral("yyyy"));
    return dt;
}

QDateTime Rct2TicketPrivate::parseTime(const QString &dateStr, const QString &timeStr) const
{
    const auto d = QDate::fromString(dateStr, QStringLiteral("dd.MM"));
    auto t = QTime::fromString(timeStr, QStringLiteral("hh:mm"));
    if (!t.isValid()) {
        t = QTime::fromString(timeStr, QStringLiteral("hh.mm"));
    }

    const auto validDt = firstDayOfValidity();
    const auto year = validDt.isValid() ? validDt.year() : contextDt.date().year();

    return QDateTime({year, d.month(), d.day()}, t);
}


// 6x "U_TLAY"
// 2x version (always "01")
// 4x record length, numbers as ASCII text
// 4x ticket layout type ("RCT2")
// 4x field count
// Nx fields (see Rct2TicketField)
Rct2Ticket::Rct2Ticket()
    : d(new Rct2TicketPrivate)
{
}

Rct2Ticket::Rct2Ticket(const Uic9183TicketLayout &layout)
    : d(new Rct2TicketPrivate)
{
    d->layout = layout;
}

Rct2Ticket::Rct2Ticket(const Rct2Ticket&) = default;
Rct2Ticket::~Rct2Ticket() = default;
Rct2Ticket& Rct2Ticket::operator=(const Rct2Ticket&) = default;

bool Rct2Ticket::isValid() const
{
    return d->layout.isValid() && d->layout.type() == QLatin1String("RCT2");
}

void Rct2Ticket::setContextDate(const QDateTime &contextDt)
{
    d->contextDt = contextDt;
}

QDate Rct2Ticket::firstDayOfValidity() const
{
    return d->firstDayOfValidity();
}

static const struct {
    const char *name; // case folded
    Rct2Ticket::Type type;
} rct2_ticket_type_map[] = {
    { "ticket + reservation", Rct2Ticket::TransportReservation },
    { "fahrschein + reservierung", Rct2Ticket::TransportReservation },
    { "ticket", Rct2Ticket::Transport },
    { "billet", Rct2Ticket::Transport },
    { "fahrkarte", Rct2Ticket::Transport },
    { "fahrschein", Rct2Ticket::Transport },
    { "reservation", Rct2Ticket::Reservation },
    { "reservierung", Rct2Ticket::Reservation },
};

Rct2Ticket::Type Rct2Ticket::type() const
{
    // in theory: columns 15 - 18 blank, columns 19 - 51 ticket type (1-based indices!)
    // however, some providers overrun and also use the blank columns, so consider those too
    // if they are really empty, we trim them anyway.
    const auto typeName1 = d->layout.text(0, 14, 38, 1).trimmed().toCaseFolded();
    const auto typeName2 = d->layout.text(1, 14, 38, 1).trimmed().toCaseFolded(); // used for alternative language type name

    // prefer exact matches
    for (auto it = std::begin(rct2_ticket_type_map); it != std::end(rct2_ticket_type_map); ++it) {
        if (typeName1 == QLatin1String(it->name) || typeName2 == QLatin1String(it->name)) {
            return it->type;
        }
    }
    for (auto it = std::begin(rct2_ticket_type_map); it != std::end(rct2_ticket_type_map); ++it) {
        if (typeName1.contains(QLatin1String(it->name)) || typeName2.contains(QLatin1String(it->name))) {
            return it->type;
        }
    }

    return Unknown;
}

QString Rct2Ticket::passengerName() const
{
    return d->layout.text(0, 52, 19, 1).trimmed();
}

QDateTime Rct2Ticket::outboundDepartureTime() const
{
    return d->parseTime(d->layout.text(6, 1, 5, 1), d->layout.text(6, 7, 5, 1));
}

QDateTime Rct2Ticket::outboundArrivalTime() const
{
    return d->parseTime(d->layout.text(6, 52, 5, 1), d->layout.text(6, 58, 5, 1));
}

QString Rct2Ticket::outboundDepartureStation() const
{
    const auto s = d->layout.text(6, 13, 17, 1).trimmed();
    if (s == QLatin1Char('*')) { // * is used to mark unset fields
        return {};
    }
    return s;
}

QString Rct2Ticket::outboundArrivalStation() const
{
    const auto s = d->layout.text(6, 34, 17, 1).trimmed();
    if (s == QLatin1Char('*')) { // * is used to mark unset fields
        return {};
    }
    return s;
}

QString Rct2Ticket::outboundClass() const
{
    return d->layout.text(6, 66, 5, 1).trimmed();
}

QString Rct2Ticket::trainNumber() const
{
    const auto t = type();
    if (t == Reservation || t == TransportReservation) {
        const auto cat = d->layout.text(8, 13, 3, 1).trimmed();
        auto num = d->layout.text(8, 7, 5, 1).trimmed();

        // check for train number bleeding into our left neighbour field (happens e.g. on ÖBB IRT/RES tickets)
        if (num.isEmpty() || num.at(0).isDigit()) {
            const auto numPrefix = d->layout.text(8, 1, 6, 1);
            for (int i = numPrefix.size() - 1; i >= 0; --i) {
                if (numPrefix.at(i).isDigit()) {
                    num.prepend(numPrefix.at(i));
                } else {
                    break;
                }
            }
        }
        num = num.trimmed();

        if (!cat.isEmpty()) {
            return cat + QLatin1Char(' ') + num;
        }
        return num;
    }
    return {};
}

QString Rct2Ticket::coachNumber() const
{
    const auto t = type();
    if (t == Reservation || t == TransportReservation) {
        return d->layout.text(8, 26, 3, 1).trimmed();
    }
    return {};
}

QString Rct2Ticket::seatNumber() const
{
    const auto t = type();
    if (t == Reservation || t == TransportReservation) {
        const auto row8 = d->layout.text(8, 48, 23, 1).trimmed();
        if (!row8.isEmpty()) {
            return row8;
        }
        // rows 9/10 can contain seating details, let's use those as fallback if we don't find a number in the right field
        return d->layout.text(9, 32, 19, 2).simplified();
    }
    return {};
}
