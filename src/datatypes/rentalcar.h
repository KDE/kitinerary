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

#ifndef KITINERARY_RENTALCAR_H
#define KITINERARY_RENTALCAR_H

#include "kitinerary_export.h"
#include "datatypes.h"

namespace KItinerary {

class RentalCarPrivate;

/** An event.
 *  @see https://developers.google.com/gmail/markup/reference/event-reservation
 */
class KITINERARY_EXPORT RentalCar
{
    KITINERARY_GADGET(RentalCar)
    KITINERARY_PROPERTY(QString, name, setName)
    KITINERARY_PROPERTY(QString, model, setModel)
    //Add more info : brand/rentalcompany
private:
    QExplicitlySharedDataPointer<RentalCarPrivate> d;
};

}

Q_DECLARE_METATYPE(KItinerary::RentalCar)

#endif // KITINERARY_RENTALCAR_H
