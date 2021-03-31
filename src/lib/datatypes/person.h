/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kitinerary_export.h"
#include "datatypes.h"

namespace KItinerary {

class PersonPrivate;

/** A person
 *  @see https://schema.org/Person
 */
class KITINERARY_EXPORT Person
{
    KITINERARY_GADGET(Person)
    KITINERARY_PROPERTY(QString, name, setName)
    KITINERARY_PROPERTY(QString, email, setEmail)
    KITINERARY_PROPERTY(QString, familyName, setFamilyName)
    KITINERARY_PROPERTY(QString, givenName, setGivenName)
private:
    QExplicitlySharedDataPointer<PersonPrivate> d;
};

}

Q_DECLARE_METATYPE(KItinerary::Person)

