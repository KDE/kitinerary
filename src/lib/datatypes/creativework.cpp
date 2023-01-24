/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "creativework.h"
#include "datatypes_p.h"

using namespace KItinerary;

namespace KItinerary {

class CreativeWorkPrivate : public QSharedData
{
    KITINERARY_PRIVATE_BASE_GADGET(CreativeWork)
public:
    QString name;
    QString description;
    QString encodingFormat;
};

KITINERARY_MAKE_CLASS(CreativeWork)
KITINERARY_MAKE_PROPERTY(CreativeWork, QString, name, setName)
KITINERARY_MAKE_PROPERTY(CreativeWork, QString, description, setDescription)
KITINERARY_MAKE_PROPERTY(CreativeWork, QString, encodingFormat, setEncodingFormat)
KITINERARY_MAKE_OPERATOR(CreativeWork)

class DigitalDocumentPrivate : public CreativeWorkPrivate
{
    KITINERARY_PRIVATE_GADGET(DigitalDocument)
};
KITINERARY_MAKE_DERIVED_CLASS(DigitalDocument, CreativeWork)
KITINERARY_MAKE_OPERATOR(DigitalDocument)

class EmailMessagePrivate : public CreativeWorkPrivate
{
    KITINERARY_PRIVATE_GADGET(EmailMessage)
};
KITINERARY_MAKE_DERIVED_CLASS(EmailMessage, CreativeWork)
KITINERARY_MAKE_OPERATOR(EmailMessage)

}

template <>
KItinerary::CreativeWorkPrivate *QExplicitlySharedDataPointer<KItinerary::CreativeWorkPrivate>::clone()
{
    return d->clone();
}

#include "moc_creativework.cpp"
