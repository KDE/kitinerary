/*
    SPDX-FileCopyrightText: 2018 Benjamin Port <benjamin.port@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

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

