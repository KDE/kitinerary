/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "rct2ticket.h"
#include "logging.h"
#include "uic9183ticketlayout.h"

#include <QDateTime>
#include <QDebug>
#include <QRegularExpression>

#include <cstring>

using namespace KItinerary;

namespace KItinerary {

class Rct2TicketPrivate : public QSharedData
{
public:
    QDate firstDayOfValidity() const;
    QDateTime parseTime(const QString &dateStr, const QString &timeStr) const;
    QString reservationPatternCapture(QStringView name) const;

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
    const auto dtStr = QStringView(f).mid(std::distance(f.begin(), it));
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
    auto d = QDate::fromString(dateStr, QStringLiteral("dd.MM"));
    if (!d.isValid()) {
        d = QDate::fromString(dateStr, QStringLiteral("dd/MM"));
    }
    if (!d.isValid()) {
        d = QDate::fromString(dateStr, QStringLiteral("dd-MM"));
    }
    auto t = QTime::fromString(timeStr, QStringLiteral("hh:mm"));
    if (!t.isValid()) {
        t = QTime::fromString(timeStr, QStringLiteral("hh.mm"));
    }

    const auto validDt = firstDayOfValidity();
    const auto baseDate = validDt.isValid() ? validDt : contextDt.date();
    auto dt = QDateTime({baseDate.year(), d.month(), d.day()}, t);
    if (dt.isValid() && dt.date() < baseDate) {
        dt = dt.addYears(1);
    }
    return dt;
}

static constexpr const char* res_patterns[] = {
    "ZUG +(?P<train_number>\\d+) +(?P<train_category>[A-Z][A-Z0-9]+) +WAGEN +(?P<coach>\\d+) +PLATZ +(?P<seat>\\d[\\d, ]+)",
    "ZUG +(?P<train_number>\\d+) +WAGEN +(?P<coach>\\d+) +PLATZ +(?P<seat>\\d[\\d, ]+)",
};

QString Rct2TicketPrivate::reservationPatternCapture(QStringView name) const
{
    const auto text = layout.text(8, 0, 72, 1);
    for (const auto *pattern : res_patterns) {
        QRegularExpression re{QLatin1String(pattern), QRegularExpression::CaseInsensitiveOption};
        Q_ASSERT(re.isValid());
        const auto match = re.match(text);
        if (match.hasMatch()) {
            return match.captured(name);
        }
    }
    return {};
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

static constexpr const struct {
    const char *name; // case folded
    Rct2Ticket::Type type;
} rct2_ticket_type_map[] = {
    { "ticket+reservation", Rct2Ticket::TransportReservation },
    { "fahrschein+reservierung", Rct2Ticket::TransportReservation },
    { "menetjegy+helyjegy", Rct2Ticket::TransportReservation },
    { "upgrade", Rct2Ticket::Upgrade },
    { "aufpreis", Rct2Ticket::Upgrade },
    { "ticket", Rct2Ticket::Transport },
    { "billet", Rct2Ticket::Transport },
    { "fahrkarte", Rct2Ticket::Transport },
    { "fahrschein", Rct2Ticket::Transport },
    { "cestovny listok", Rct2Ticket::Transport },
    { "jizdenka", Rct2Ticket::Transport },
    { "menetjegy", Rct2Ticket::Transport },
    { "reservation", Rct2Ticket::Reservation },
    { "reservierung", Rct2Ticket::Reservation },
    { "helyjegy", Rct2Ticket::Reservation },
    { "interrail", Rct2Ticket::RailPass },
};

Rct2Ticket::Type Rct2Ticket::type() const
{
    // in theory: columns 15 - 18 blank, columns 19 - 51 ticket type (1-based indices!)
    // however, some providers overrun and also use the blank columns, so consider those too
    // if they are really empty, we trim them anyway.
    const auto typeName1 = d->layout.text(0, 14, 38, 1).trimmed().remove(QLatin1Char(' ')).toCaseFolded();
    const auto typeName2 = d->layout.text(1, 14, 38, 1).trimmed().remove(QLatin1Char(' ')).toCaseFolded(); // used for alternative language type name

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

    // alternatively, check all fields covering the title area, for even more creative placements...
    for (const auto &f : d->layout.fields(0, 14, 38, 2)) {
        for (auto it = std::begin(rct2_ticket_type_map); it != std::end(rct2_ticket_type_map); ++it) {
            if (f.text().toCaseFolded().contains(QLatin1String(it->name))) {
                return it->type;
            }
        }
    }

    return Unknown;
}

QString Rct2Ticket::title() const
{
    // RPT has shorter title fields
    if (type() == Rct2Ticket::RailPass) {
        return d->layout.text(0, 18, 19, 1);
    }

    // somewhat standard compliant layout
    if (d->layout.text(0, 15, 3, 1).trimmed().isEmpty()) {
        const auto s = d->layout.text(0, 18, 33, 1).trimmed();
        return s.isEmpty() ? d->layout.text(1, 18, 33, 1).trimmed() : s;
    }

    // "creative" layout
    return d->layout.text(0, 0, 52, 1).trimmed();
}

QString Rct2Ticket::passengerName() const
{
    const auto name = d->layout.text(0, 52, 19, 1).trimmed();
    // sanity-check if this is a plausible name, e.g. Renfe has random other stuff here
    return std::any_of(name.begin(), name.end(), [](QChar c) { return c.isDigit(); }) ? QString() : name;
}

QDateTime Rct2Ticket::outboundDepartureTime() const
{
    return d->parseTime(d->layout.text(6, 1, 5, 1).trimmed(), d->layout.text(6, 7, 5, 1).trimmed());
}

QDateTime Rct2Ticket::outboundArrivalTime() const
{
    auto dt = d->parseTime(d->layout.text(6, 52, 5, 1).trimmed(), d->layout.text(6, 58, 5, 1).trimmed());
    if (dt.isValid() && dt < outboundDepartureTime()) {
        dt = dt.addYears(1);
    }
    return dt;
}

static QString rct2Clean(const QString &s)
{
    // * is used to mark unset fields
    if (std::all_of(s.begin(), s.end(), [](QChar c) { return c == QLatin1Char('*'); })) {
        return {};
    }
    return s;
}

QString Rct2Ticket::outboundDepartureStation() const
{
    if (type() == RailPass) {
        return {};
    }

    // 6, 13, 17, 1 would be according to spec, but why stick to that...
    const auto fields = d->layout.containedFields(6, 13, 17, 1);
    if (fields.size() == 1) {
        return rct2Clean(fields[0].text().trimmed());
    }
    return rct2Clean(d->layout.text(6, 12, 18, 1).trimmed());
}

QString Rct2Ticket::outboundArrivalStation() const
{
    return type() != RailPass ? rct2Clean(d->layout.text(6, 34, 17, 1).trimmed()) : QString();
}

QString Rct2Ticket::outboundClass() const
{
    return rct2Clean(d->layout.text(6, 66, 5, 1).trimmed());
}

QDateTime Rct2Ticket::returnDepartureTime() const
{
    return d->parseTime(d->layout.text(7, 1, 5, 1).trimmed(), d->layout.text(7, 7, 5, 1).trimmed());
}

QDateTime Rct2Ticket::returnArrivalTime() const
{
    auto dt = d->parseTime(d->layout.text(7, 52, 5, 1).trimmed(), d->layout.text(7, 58, 5, 1).trimmed());
    if (dt.isValid() && dt < returnDepartureTime()) {
        dt = dt.addYears(1);
    }
    return dt;
}

QString Rct2Ticket::returnDepartureStation() const
{
    // 7, 13, 17, 1 would be according to spec, but you can guess by now how well that is followed...
    return type() != RailPass ? rct2Clean(d->layout.text(7, 12, 18, 1).trimmed()) : QString();
}

QString Rct2Ticket::returnArrivalStation() const
{
    return type() != RailPass ? rct2Clean(d->layout.text(7, 34, 17, 1).trimmed()) : QString();
}

QString Rct2Ticket::returnClass() const
{
    return rct2Clean(d->layout.text(7, 66, 5, 1).trimmed());
}

QString Rct2Ticket::trainNumber() const
{
    const auto t = type();
    if (t == Reservation || t == TransportReservation || t == Upgrade) {
        auto num = d->reservationPatternCapture(u"train_number");
        if (!num.isEmpty()) {
            return d->reservationPatternCapture(u"train_category") + QLatin1Char(' ') + num;
        }

        const auto cat = d->layout.text(8, 13, 3, 1).trimmed();
        num = d->layout.text(8, 7, 5, 1).trimmed();

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
        const auto coach = d->reservationPatternCapture(u"coach");
        return coach.isEmpty() ? d->layout.text(8, 26, 3, 1).trimmed() : coach;
    }
    return {};
}

QString Rct2Ticket::seatNumber() const
{
    const auto t = type();
    if (t == Reservation || t == TransportReservation) {
        const auto seat = d->reservationPatternCapture(u"seat");
        if (!seat.isEmpty()) {
            return seat;
        }

        const auto row8 = d->layout.text(8, 48, 23, 1).trimmed();
        if (!row8.isEmpty()) {
            return row8;
        }
        // rows 9/10 can contain seating details, let's use those as fallback if we don't find a number in the right field
        return d->layout.text(9, 32, 19, 2).simplified();
    }
    return {};
}
