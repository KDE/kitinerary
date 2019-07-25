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

#ifndef KITINERARY_CREATIVEWORK_H
#define KITINERARY_CREATIVEWORK_H

#include "kitinerary_export.h"
#include "datatypes.h"

class QUrl;

namespace KItinerary {

class CreativeWorkPrivate;

/** Base type describing any form of content.
 *  @see https://schema.org/CreativeWork
 */
class KITINERARY_EXPORT CreativeWork
{
    KITINERARY_BASE_GADGET(CreativeWork)
    /** Name of the file. */
    KITINERARY_PROPERTY(QString, name, setName)
    /** Human readable description. */
    KITINERARY_PROPERTY(QString, description, setDescription)
    /** Mimetype. */
    KITINERARY_PROPERTY(QString, encodingFormat, setEncodingFormat)

protected:
    ///@cond internal
    QExplicitlySharedDataPointer<CreativeWorkPrivate> d;
    ///@endcond
};

/** Description of a document.
 *  @see https://schema.org/DigitalDocument
 */
class KITINERARY_EXPORT DigitalDocument : public CreativeWork
{
    KITINERARY_GADGET(DigitalDocument)
};

/** Description of an email.
 *  @see https://schema.org/EmailMessage
 */
class KITINERARY_EXPORT EmailMessage : public CreativeWork
{
    KITINERARY_GADGET(EmailMessage)
};

}

Q_DECLARE_METATYPE(KItinerary::CreativeWork)
Q_DECLARE_METATYPE(KItinerary::DigitalDocument)
Q_DECLARE_METATYPE(KItinerary::EmailMessage)

#endif // KITINERARY_CREATIVEWORK_H

