/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_EXTRACTORINPUT_H
#define KITINERARY_EXTRACTORINPUT_H

#include "kitinerary_export.h"

#include <qobjectdefs.h>

class QByteArray;
class QString;

namespace KItinerary {

/** Extractor input data type methods. */
namespace ExtractorInput
{
    KITINERARY_EXPORT Q_NAMESPACE
    /** Type of extractor input data. */
    enum Type {
        Unknown = 0, ///< Unknown or not yet detected input type.
        Text, ///< Plain text document.
        Barcode, ///< Content of a barcode.
        Email, ///< An email message.
        Html, ///< A HTML document.
        JsonLd, ///< JSON-LD encoded schema.org structured data.
        ICal, ///< An iCalendar event.
        Pdf, ///< A PDF file.
        PkPass ///< An Apple Wallet pass file.
    };
    Q_ENUM_NS(Type)

    /** Try to determine data type based on @p content. */
    KITINERARY_EXPORT Type typeFromContent(const QByteArray &content);
    /** Return the content type based on the given MIME type. */
    KITINERARY_EXPORT Type typeFromMimeType(const QString &mimeType);
    /** Try to determine data type based on the file name. */
    KITINERARY_EXPORT Type typeFromFileName(const QString &fileName);
    /** Convert type enum to a string. */
    KITINERARY_EXPORT QString typeToString(Type type);
    /** Convert string representation of the type to an enum. */
    KITINERARY_EXPORT Type typeFromName(const QString &name);
}

}

#endif // KITINERARY_EXTRACTORINPUT_H
