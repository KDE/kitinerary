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

#include "uic9183ticketlayout.h"
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
class Uic9183TicketLayoutField
{
public:
    Uic9183TicketLayoutField() = default;
    /** Create a new U_TLAY field starting at @p data.
     *  @param size The size of the remaining U_TLAY field array (not just this field!).
     */
    Uic9183TicketLayoutField(const char *data, int size);
    bool isNull() const;
    // size of the field data, not size of the text content
    int size() const;

    int row() const;
    int column() const;
    int height() const;
    int width() const;
    QString text() const;

    Uic9183TicketLayoutField next() const;

private:
    const char *m_data = nullptr;
    int m_size = 0;
};

class Uic9183TicketLayoutPrivate : public QSharedData
{
public:
    Uic9183TicketLayoutField firstField() const;

    int size = 0;
    const char *data = nullptr;
};

}


Uic9183TicketLayoutField::Uic9183TicketLayoutField(const char *data, int size)
    : m_data(data)
    , m_size(size)
{
    if (size <= 13) { // too small
        qCWarning(Log) << "Found too small U_TLAY field:" << size;
        m_data = nullptr;
        return;
    }

    // invalid format
    if (!std::all_of(data, data + 8, isdigit) || !std::all_of(data + 9, data + 13, isdigit)) {
        qCWarning(Log) << "Found U_TLAY field with invalid format";
        m_data = nullptr;
        return;
    }

    // size is too large
    if (this->size() > m_size) {
        qCWarning(Log) << "Found U_TLAY field with invalid size" << this->size() << m_size;
        m_data = nullptr;
        return;
    }
}

bool Uic9183TicketLayoutField::isNull() const
{
    return !m_data || m_size <= 13;
}

int Uic9183TicketLayoutField::size() const
{
    return asciiToInt(m_data + 9, 4) + 13;
}

int Uic9183TicketLayoutField::row() const
{
    return asciiToInt(m_data, 2);
}

int Uic9183TicketLayoutField::column() const
{
    return asciiToInt(m_data + 2, 2);
}

int Uic9183TicketLayoutField::height() const
{
    return asciiToInt(m_data + 4, 2);
}

int Uic9183TicketLayoutField::width() const
{
    return asciiToInt(m_data + 6, 2);
}

QString Uic9183TicketLayoutField::text() const
{
    return QString::fromUtf8(m_data + 13, asciiToInt(m_data + 9, 4));
}

Uic9183TicketLayoutField Uic9183TicketLayoutField::next() const
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
        Uic9183TicketLayoutField f(m_data + thisSize + i, remaining - i);
        if (!f.isNull()) {
            return f;
        }
    }

    return {};
}

Uic9183TicketLayoutField Uic9183TicketLayoutPrivate::firstField() const
{
    if (size > 20) {
        return Uic9183TicketLayoutField(data + 20, size - 20);
    }
    return {};
}


// 6x "U_TLAY"
// 2x version (always "01")
// 4x record length, numbers as ASCII text
// 4x ticket layout type (e.g. "RCT2")
// 4x field count
// Nx fields (see Uic9183TicketLayoutField)
Uic9183TicketLayout::Uic9183TicketLayout()
    : d(new Uic9183TicketLayoutPrivate)
{
}

Uic9183TicketLayout::Uic9183TicketLayout(const char *data, int size)
    : d(new Uic9183TicketLayoutPrivate)
{
    d->data = data;
    d->size = size;

#if 0
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
#endif
}

Uic9183TicketLayout::Uic9183TicketLayout(const Uic9183TicketLayout&) = default;
Uic9183TicketLayout::~Uic9183TicketLayout() = default;
Uic9183TicketLayout& Uic9183TicketLayout::operator=(const Uic9183TicketLayout&) = default;

QString Uic9183TicketLayout::type() const
{
    return QString::fromUtf8(d->data + 12, 4);
}

bool Uic9183TicketLayout::isValid() const
{
    return d->data && d->size > 34 && std::strncmp(d->data + 6, "01", 2) == 0;
}

QString Uic9183TicketLayout::text(int row, int column, int width, int height) const
{
    QStringList s;
    s.reserve(height);
    for (int i = 0; i < height; ++i) {
        s.push_back({});
    }

    for (auto f = d->firstField(); !f.isNull(); f = f.next()) {
        if (f.row() + f.height() - 1 < row || f.row() > row + height - 1) {
            continue;
        }
        if (f.column() + f.width() - 1 < column || f.column() > column + width - 1) {
            continue;
        }
        //qDebug() << "Field:" << f.row() << f.column() << f.height() << f.width() << f.size() << f.text();

        // split field into lines
        // TODO this needs to follow the U_TLAY word-wrapping algorithm?
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
