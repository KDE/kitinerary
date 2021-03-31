/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kitinerary_export.h"
#include "datatypes.h"


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


