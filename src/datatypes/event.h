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

#ifndef KITINERARY_EVENT_H
#define KITINERARY_EVENT_H

#include "kitinerary_export.h"
#include "datatypes.h"

namespace KItinerary {

class EventPrivate;

/** An event.
 *  @see https://schema.org/Event
 *  @see https://developers.google.com/gmail/markup/reference/event-reservation
 */
class KITINERARY_EXPORT Event
{
    KITINERARY_GADGET(Event)
    KITINERARY_PROPERTY(QString, name, setName)
    KITINERARY_PROPERTY(QUrl, url, setUrl)
    KITINERARY_PROPERTY(QDateTime, startDate, setStartDate)
    KITINERARY_PROPERTY(QDateTime, endDate, setEndDate)
    KITINERARY_PROPERTY(QDateTime, doorTime, setDoorTime)
    KITINERARY_PROPERTY(QVariant, location, setLocation)

private:
    QExplicitlySharedDataPointer<EventPrivate> d;
};

}

Q_DECLARE_METATYPE(KItinerary::Event)

#endif // KITINERARY_EVENT_H
