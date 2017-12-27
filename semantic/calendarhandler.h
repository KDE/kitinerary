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

#ifndef CALENDARHANDLER_H
#define CALENDARHANDLER_H

#include <KCalCore/Event>

class QVariant;

/** Methods for converting between ical events and JSON-LD booking data. */
class CalendarHandler
{
public:
    static void fillEvent(const QVariant &reservation, const KCalCore::Event::Ptr &event);

private:
    static void fillFlightReservation(const QVariant &reservation, const KCalCore::Event::Ptr &event);
    static void fillTrainReservation(const QVariant &reservation, const KCalCore::Event::Ptr &event);
    static void fillLodgingReservation(const QVariant &reservation, const KCalCore::Event::Ptr &event);
    static void fillGeoPosition(const QVariant &place, const KCalCore::Event::Ptr &event);
};

#endif // CALENDARHANDLER_H
