/*
   Copyright (c) 2017 Volker Krause <vkrause@kde.org>

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   This library is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
   License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.
*/

#include "datatypes.h"

#include <QLocale>
#include <QTimeZone>

bool Airport::operator!=(const Airport &other) const
{
    return m_iataCode != other.m_iataCode || m_name != other.m_name;
}

bool Airline::operator!=(const Airline &other) const
{
    return m_iataCode != other.m_iataCode || m_name != other.m_name;
}

static QString localizedDateTime(const QDateTime &dt)
{
    auto s = QLocale().toString(dt, QLocale::ShortFormat);
    if (dt.timeSpec() == Qt::TimeZone || dt.timeSpec() == Qt::OffsetFromUTC) {
        s += QLatin1Char(' ') + dt.timeZone().abbreviation(dt);
    }
    return s;
}

QString Flight::departureTimeLocalized() const
{
    return localizedDateTime(m_departureTime);
}

QString Flight::arrivalTimeLocalized() const
{
    return localizedDateTime(m_arrivalTime);
}

QString LodgingReservation::checkinDateLocalized() const
{
    return QLocale().toString(m_checkinDate.date(), QLocale::ShortFormat);
}

QString LodgingReservation::checkoutDateLocalized() const
{
    return QLocale().toString(m_checkoutDate.date(), QLocale::ShortFormat);
}
