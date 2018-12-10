/*
    Copyright (C) 2018 Benjamin Port <benjamin.port@kde.org>

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

#ifndef KITINERARY_BRAND_H
#define KITINERARY_BRAND_H

#include "kitinerary_export.h"
#include "datatypes.h"

namespace KItinerary {

class BrandPrivate;

/** A brand
 *  @see https://schema.org/Brand
 */
class KITINERARY_EXPORT Brand
{
    KITINERARY_GADGET(Brand)
    KITINERARY_PROPERTY(QString, name, setName)

private:
    QExplicitlySharedDataPointer<BrandPrivate> d;
};

}

Q_DECLARE_METATYPE(KItinerary::Brand)

#endif // KITINERARY_BRAND_H
