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

#ifndef KITINERARY_DOCUMENTUTIL_H
#define KITINERARY_DOCUMENTUTIL_H

#include "kitinerary_export.h"

#include <QVariant>

namespace KItinerary {

/** Utilities for dealing with documents attached to reservations. */
namespace DocumentUtil
{
    /** Determine a document identifier for the given content. */
    KITINERARY_EXPORT QString idForContent(const QByteArray &data);

    /** Add a document id to the reservation @p res.
     *  @returns @c true if the document id wasn't present yet and the reservation changed, @p false otherwise.
     */
    KITINERARY_EXPORT bool addDocumentId(QVariant &res, const QString &id);

    /** Removes the document identifier @p id from reservation @p res.
     *  @returns @c true if the document id was present and the reservation changed, @p false otherwise.
     */
    KITINERARY_EXPORT bool removeDocumentId(QVariant &res, const QString &id);

    /** Returns all document identifiers associcated with reservation @p res. */
    KITINERARY_EXPORT QVariantList documentIds(const QVariant &res);

    /** Sets the list of document identifiers for reservation @p res. */
    KITINERARY_EXPORT void setDocumentIds(QVariant &res, const QVariantList &docIds);
}

}

#endif // KITINERARY_DOCUMENTUTIL_H
