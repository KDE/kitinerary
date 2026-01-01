/*
    SPDX-FileCopyrightText: 2018-2026 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "taxi.h"
#include "datatypes_p.h"

#include <QDateTime>
#include <QUrl>

using namespace KItinerary;

namespace KItinerary {

class TaxiPrivate: public QSharedData {

public:
    QString name;
};

KITINERARY_MAKE_CLASS(Taxi)
KITINERARY_MAKE_PROPERTY(Taxi, QString, name, setName)
KITINERARY_MAKE_OPERATOR(Taxi)
}

#include "moc_taxi.cpp"
