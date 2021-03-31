/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

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
    KITINERARY_PROPERTY(QString, description, setDescription)
    KITINERARY_PROPERTY(QUrl, image, setImage)
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

