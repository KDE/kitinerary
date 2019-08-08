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
        Unknown = 0,
        Text,
        Email,
        Html,
        JsonLd,
        ICal,
        Pdf,
        PkPass
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
