/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

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

KITINERARY_MAKE_BASE_CLASS(CreativeWork)
KITINERARY_MAKE_PROPERTY(CreativeWork, QString, name, setName)
KITINERARY_MAKE_PROPERTY(CreativeWork, QString, description, setDescription)
KITINERARY_MAKE_PROPERTY(CreativeWork, QString, encodingFormat, setEncodingFormat)
KITINERARY_MAKE_OPERATOR(CreativeWork)

class DigitalDocumentPrivate : public CreativeWorkPrivate
{
    KITINERARY_PRIVATE_GADGET(DigitalDocument)
};
KITINERARY_MAKE_SUB_CLASS(DigitalDocument, CreativeWork)
KITINERARY_MAKE_OPERATOR(DigitalDocument)

class EmailMessagePrivate : public CreativeWorkPrivate
{
    KITINERARY_PRIVATE_GADGET(EmailMessage)
};
KITINERARY_MAKE_SUB_CLASS(EmailMessage, CreativeWork)
KITINERARY_MAKE_OPERATOR(EmailMessage)

}

template <>
KItinerary::CreativeWorkPrivate *QExplicitlySharedDataPointer<KItinerary::CreativeWorkPrivate>::clone()
{
    return d->clone();
}

#include "moc_creativework.cpp"
