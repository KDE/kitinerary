/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "uic9183ticketlayout.h"
#include "uic9183block.h"
#include "logging.h"

#include <QDateTime>
#include <QDebug>
#include <QSize>

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

class Uic9183TicketLayoutPrivate : public QSharedData
{
public:
    Uic9183Block block;
};

}

Uic9183TicketLayoutField::Uic9183TicketLayoutField() = default;

// 2x field line, number as ascii text
// 2x field column
// 2x field height
// 2x field width
// 1x field format
// 4x text length
// Nx text content
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

int Uic9183TicketLayoutField::format() const
{
    return asciiToInt(m_data + 12, 1);
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

Uic9183TicketLayoutField Uic9183TicketLayout::firstField() const
{
    const auto contentSize = d->block.contentSize();
    if (contentSize > 8) {
        return Uic9183TicketLayoutField(d->block.content() + 8, contentSize - 8);
    }
    return {};
}


// "U_TLAY" Block
// 4x ticket layout type (e.g. "RCT2")
// 4x field count
// Nx fields (see Uic9183TicketLayoutField)
Uic9183TicketLayout::Uic9183TicketLayout()
    : d(new Uic9183TicketLayoutPrivate)
{
}

Uic9183TicketLayout::Uic9183TicketLayout(const Uic9183Block &block)
    : d(new Uic9183TicketLayoutPrivate)
{
    d->block = block;

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
    return d->block.readUtf8String(0, 4);
}

bool Uic9183TicketLayout::isValid() const
{
    return !d->block.isNull() && d->block.contentSize() > 8 && d->block.version() == 1;
}

QString Uic9183TicketLayout::text(int row, int column, int width, int height) const
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

QSize Uic9183TicketLayout::size() const
{
    int width = 0, height = 0;
    for (auto f = firstField(); !f.isNull(); f  = f.next()) {
        width = std::max(width, f.column() + f.width());
        height = std::max(height, f.row() + f.height());
    }
    return QSize(width, height);
}