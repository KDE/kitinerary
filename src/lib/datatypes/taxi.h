/*
    SPDX-FileCopyrightText: 2018-2021 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

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

