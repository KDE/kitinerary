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

#include "event.h"
#include "datatypes_p.h"

#include <QDateTime>
#include <QUrl>

using namespace KItinerary;

namespace KItinerary {

class EventPrivate: public QSharedData {

public:
    QString name;
    QString description;
    QUrl image;
    QUrl url;
    QDateTime startDate;
    QDateTime endDate;
    QDateTime doorTime;
    QVariant location;
};

KITINERARY_MAKE_SIMPLE_CLASS(Event)
KITINERARY_MAKE_PROPERTY(Event, QString, name, setName)
KITINERARY_MAKE_PROPERTY(Event, QString, description, setDescription)
KITINERARY_MAKE_PROPERTY(Event, QUrl, image, setImage)
KITINERARY_MAKE_PROPERTY(Event, QUrl, url, setUrl)
KITINERARY_MAKE_PROPERTY(Event, QDateTime, startDate, setStartDate)
KITINERARY_MAKE_PROPERTY(Event, QDateTime, endDate, setEndDate)
KITINERARY_MAKE_PROPERTY(Event, QDateTime, doorTime, setDoorTime)
KITINERARY_MAKE_PROPERTY(Event, QVariant, location, setLocation)
KITINERARY_MAKE_OPERATOR(Event)

}

#include "moc_event.cpp"
