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
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef KITINERARY_TAXI_H
#define KITINERARY_TAXI_H

#include "kitinerary_export.h"
#include "datatypes.h"

namespace KItinerary {

class TaxiPrivate;

class KITINERARY_EXPORT Taxi
{
    KITINERARY_GADGET(Taxi)
    KITINERARY_PROPERTY(QString, name, setName)
private:
    QExplicitlySharedDataPointer<TaxiPrivate> d;
};

}

Q_DECLARE_METATYPE(KItinerary::Taxi)

#endif // KITINERARY_TAXI_H
