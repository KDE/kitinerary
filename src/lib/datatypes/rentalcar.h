/*
    SPDX-FileCopyrightText: 2018-2022 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kitinerary_export.h"
#include "datatypes.h"
#include "organization.h"
#include "brand.h"

namespace KItinerary {

class RentalCarPrivate;

/** A car rental.
 *  @see https://developers.google.com/gmail/markup/reference/rental-car
 */
class KITINERARY_EXPORT RentalCar
{
    KITINERARY_GADGET(RentalCar)
    KITINERARY_PROPERTY(QString, name, setName)
    KITINERARY_PROPERTY(QString, model, setModel)
    KITINERARY_PROPERTY(KItinerary::Organization, rentalCompany, setRentalCompany)
    KITINERARY_PROPERTY(KItinerary::Brand, brand, setBrand)

private:
    QExplicitlySharedDataPointer<RentalCarPrivate> d;
};

}

Q_DECLARE_METATYPE(KItinerary::RentalCar)

