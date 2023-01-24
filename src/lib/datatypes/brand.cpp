/*
    SPDX-FileCopyrightText: 2018 Benjamin Port <benjamin.port@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "brand.h"
#include "datatypes_p.h"

using namespace KItinerary;

namespace KItinerary {

class BrandPrivate : public QSharedData
{
public:
    QString name;
};

KITINERARY_MAKE_CLASS(Brand)
KITINERARY_MAKE_PROPERTY(Brand, QString, name, setName)
KITINERARY_MAKE_OPERATOR(Brand)

}

#include "moc_brand.cpp"
