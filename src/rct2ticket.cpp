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

#include <QDateTime>
#include <QDebug>

#include <cstring>

static int asciiToInt(const char *s, int size)
{
    if (!s) {
        return 0;
    }

    int v = 0;
    for (int i = 0; i < size; ++i) {
        v *= 10;
        v += (*(s + i)) - '0';
    }
    return v;
}

using namespace KItinerary;

namespace KItinerary {

// 2x field line, number as ascii text
// 2x field column
// 2x field height
// 2x field width
// 1x field format
// 4x text length
// Nx text content
class Rct2TicketField
{
public:
    Rct2TicketField() = default;
    /** Create a new RCT2 field starting at @p data.
     *  @param size The size of the remaining RCT2 field array (not just this field!).
     */
    Rct2TicketField(const char *data, int size);
    bool isNull() const;
    // size of the field data, not size of the text content
    int size() const;

    int row() const;
    int column() const;
    int height() const;
    int width() const;
    QString text() const;

    Rct2TicketField next() const;

private:
    const char *m_data = nullptr;
    int m_size = 0;
};

class Rct2TicketPrivate : public QSharedData
{
public:
    QString fieldText(int row, int column, int width, int height = 1) const;
    QDate firstDayOfValidity() const;
    QDateTime parseTime(const QString &dateStr, const QString &timeStr) const;

    Rct2TicketField firstField() const;

    int size = 0;
    const char *data = nullptr;
    QDateTime contextDt;
};

}


Rct2TicketField::Rct2TicketField(const char *data, int size)
    : m_data(data)
    , m_size(size)
{
    if (size <= 13) { // too small
        qCWarning(Log) << "Found too small RCT2 field:" << size;
        m_data = nullptr;
        return;
    }

    // invalid format
    if (!std::all_of(data, data + 8, isdigit) || !std::all_of(data + 9, data + 13, isdigit)) {
        qCWarning(Log) << "Found RCT2 field with invalid format";
        m_data = nullptr;
        return;
    }

    // size is too large
    if (this->size() > m_size) {
        qCWarning(Log) << "Found RCT2 field with invalid size" << this->size() << m_size;
        m_data = nullptr;
        return;
    }
}

bool Rct2TicketField::isNull() const
{
    return !m_data || m_size <= 13;
}

int Rct2TicketField::size() const
{
    return asciiToInt(m_data + 9, 4) + 13;
}

int Rct2TicketField::row() const
{
    return asciiToInt(m_data, 2);
}

int Rct2TicketField::column() const
{
    return asciiToInt(m_data + 2, 2);
}

int Rct2TicketField::height() const
{
    return asciiToInt(m_data + 4, 2);
}

int Rct2TicketField::width() const
{
    return asciiToInt(m_data + 6, 2);
}

QString Rct2TicketField::text() const
{
    return QString::fromUtf8(m_data + 13, asciiToInt(m_data + 9, 4));
}

Rct2TicketField Rct2TicketField::next() const
{
    const auto thisSize = size();
    const auto remaining = m_size - size();
    if (remaining < 0) {
        return {};
    }

    // search for the next field
    // in theory this should always trigger at i == 0, unless
    // the size field is wrong, which happens unfortunately
    for (int i = 0; i < remaining - 13; ++i) {
        Rct2TicketField f(m_data + thisSize + i, remaining - i);
        if (!f.isNull()) {
            return f;
        }
    }

    return {};
}

Rct2TicketField Rct2TicketPrivate::firstField() const
{
    if (size > 20) {
        return Rct2TicketField(data + 20, size - 20);
    }
    return {};
}

QString Rct2TicketPrivate::fieldText(int row, int column, int width, int height) const
{
    QStringList s;
    s.reserve(height);
    for (int i = 0; i < height; ++i) {
        s.push_back({});
    }

    for (auto f = firstField(); !f.isNull(); f = f.next()) {
        if (f.row() + f.height() - 1 < row || f.row() > row + height - 1) {
            continue;
        }
        if (f.column() + f.width() - 1 < column || f.column() > column + width - 1) {
            continue;
        }
        //qDebug() << "Field:" << f.row() << f.column() << f.height() << f.width() << f.size() << f.text();

        // split field into lines
        // TODO this needs to follow the RCT2 word-wrapping algorithm?
        const auto content = f.text();
        const auto lines = content.splitRef(QLatin1Char('\n'));

        // cut out the right part of the line
        for (int i = 0; i < lines.size(); ++i) {
            if (f.row() + i < row) {
                continue;
            }
            if (f.row() + i > row + height - 1) {
                break;
            }

            // TODO also truncate by w
            const auto offset = column - f.column();
            if (offset >= 0) {
                s[f.row() + i - row] += lines.at(i).mid(offset).left(width);
            } else {
                s[f.row() + i - row] += lines.at(i); // TODO left padding by offset, truncate by width + offset
            }
        }
    }
    //qDebug() << "Result:" << row << column << width << height << s;
    return s.join(QLatin1Char('\n'));
}

QDate Rct2TicketPrivate::firstDayOfValidity() const
{
    const auto f = fieldText(3, 1, 48);
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

Rct2Ticket::Rct2Ticket(const char *data, int size)
    : d(new Rct2TicketPrivate)
{
    d->data = data;
    d->size = size;

    qDebug() << QByteArray(data, size);
    std::vector<QString> out;
    for (auto f = d->firstField(); !f.isNull(); f = f.next()) {
        qDebug() << "Field:" << f.row() << f.column() << f.width() << f.height() << f.text() << f.size();
        out.resize(std::max<int>(f.row() + 1, out.size()));
        out[f.row()].resize(std::max(out[f.row()].size(), f.column() + f.width() + 1), QLatin1Char(' '));
        out[f.row()].replace(f.column(), f.width(), f.text());
    }
    for (const auto &line : out) {
        qDebug() << line;
    }
}

Rct2Ticket::Rct2Ticket(const Rct2Ticket&) = default;
Rct2Ticket::~Rct2Ticket() = default;
Rct2Ticket& Rct2Ticket::operator=(const Rct2Ticket&) = default;

bool Rct2Ticket::isValid() const
{
    return d->data && d->size > 34
        && std::strncmp(d->data + 6, "01", 2) == 0
        && std::strncmp(d->data + 12, "RCT2", 4) == 0;
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
    { "reservation", Rct2Ticket::Reservation }
};

Rct2Ticket::Type Rct2Ticket::type() const
{
    // in theory: columns 15 - 18 blank, columns 19 - 51 ticket type (1-based indices!)
    // however, some providers overrun and also use the blank columns, so consider those too
    // if they are really empty, we trim them anyway.
    const auto typeName1 = d->fieldText(0, 14, 38).trimmed().toCaseFolded();
    const auto typeName2 = d->fieldText(1, 14, 38).trimmed().toCaseFolded(); // used for alternative language type name

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
    return d->fieldText(0, 52, 19).trimmed();
}

QDateTime Rct2Ticket::outboundDepartureTime() const
{
    return d->parseTime(d->fieldText(6, 1, 5), d->fieldText(6, 7, 5));
}

QDateTime Rct2Ticket::outboundArrivalTime() const
{
    return d->parseTime(d->fieldText(6, 52, 5), d->fieldText(6, 58, 5));
}

QString Rct2Ticket::outboundDepartureStation() const
{
    const auto s = d->fieldText(6, 13, 17).trimmed();
    if (s == QLatin1Char('*')) { // * is used to mark unset fields
        return {};
    }
    return s;
}

QString Rct2Ticket::outboundArrivalStation() const
{
    const auto s = d->fieldText(6, 34, 17).trimmed();
    if (s == QLatin1Char('*')) { // * is used to mark unset fields
        return {};
    }
    return s;
}

QString Rct2Ticket::outboundClass() const
{
    return d->fieldText(6, 66, 5).trimmed();
}

QString Rct2Ticket::trainNumber() const
{
    const auto t = type();
    if (t == Reservation || t == TransportReservation) {
        const auto cat = d->fieldText(8, 13, 3).trimmed();
        auto num = d->fieldText(8, 7, 5);

        // check for train number bleeding into our left neighbour field (happens e.g. on ÖBB IRT tickets)
        if (!num.isEmpty() && num.at(0).isDigit()) {
            const auto numPrefix = d->fieldText(8, 1, 6);
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
        return d->fieldText(8, 26, 3).trimmed();
    }
    return {};
}

QString Rct2Ticket::seatNumber() const
{
    const auto t = type();
    if (t == Reservation || t == TransportReservation) {
        const auto row8 = d->fieldText(8, 48, 23).trimmed();
        if (!row8.isEmpty()) {
            return row8;
        }
        // rows 9/10 can contain seating details, let's use those as fallback if we don't find a number in the right field
        return d->fieldText(9, 1, 70, 2).simplified();
    }
    return {};
}
