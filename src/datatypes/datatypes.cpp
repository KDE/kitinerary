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
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "organization.h"
#include "person.h"
#include "place.h"

#include <QCoreApplication>
#include <QMetaType>

using namespace KItinerary;

struct StartupFunction {
    StartupFunction()
    {
        // add types here that are not covered by moc's auto-registration
        qRegisterMetaType<Airline>();
        qRegisterMetaType<Airport>();
        qRegisterMetaType<Person>();
        qRegisterMetaType<Organization>();
        qRegisterMetaType<TouristAttraction>();
        qRegisterMetaType<TrainStation>();
    }
};

StartupFunction runOnce;
