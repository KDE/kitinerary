/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
