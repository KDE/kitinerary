/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "person.h"
#include "datatypes_p.h"

using namespace KItinerary;

namespace KItinerary {

class PersonPrivate : public QSharedData
{
public:
    QString name;
    QString email;
    QString familyName;
    QString givenName;
};

KITINERARY_MAKE_SIMPLE_CLASS(Person)
KITINERARY_MAKE_PROPERTY(Person, QString, name, setName)
KITINERARY_MAKE_PROPERTY(Person, QString, email, setEmail)
KITINERARY_MAKE_PROPERTY(Person, QString, familyName, setFamilyName)
KITINERARY_MAKE_PROPERTY(Person, QString, givenName, setGivenName)
KITINERARY_MAKE_OPERATOR(Person)

}

#include "moc_person.cpp"
