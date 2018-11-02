/*
    Copyright (C) 2018 Laurent Montel <montel@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

KITINERARY_MAKE_SIMPLE_CLASS(Taxi)
KITINERARY_MAKE_PROPERTY(Taxi, QString, name, setName)
KITINERARY_MAKE_OPERATOR(Taxi)
}

#include "moc_taxi.cpp"
