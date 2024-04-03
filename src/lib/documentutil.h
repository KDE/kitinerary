/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kitinerary_export.h"

#include <QVariant>

namespace KItinerary {

/** Utilities for dealing with documents attached to objects.
 *  @see https://schema.org/subjectOf
 */
namespace DocumentUtil
{
    /** Determine a document identifier for the given content. */
    KITINERARY_EXPORT QString idForContent(const QByteArray &data);

    /** Determine a document identifier for a Apple Wallet pass. */
    QString idForPkPass(const QString &passTypeIdentifier, const QString &serialNumber);

    /** Add a document id to the object @p obj.
     *  @returns @c true if the document id wasn't present yet and the object changed, @p false otherwise.
     */
    KITINERARY_EXPORT bool addDocumentId(QVariant &obj, const QString &id);

    /** Removes the document identifier @p id from object @p res.
     *  @returns @c true if the document id was present and the object changed, @p false otherwise.
     */
    KITINERARY_EXPORT bool removeDocumentId(QVariant &obj, const QString &id);

    /** Returns all document identifiers associated with object @p obj. */
    KITINERARY_EXPORT QVariantList documentIds(const QVariant &obj);

    /** Sets the list of document identifiers for object @p obj. */
    KITINERARY_EXPORT void setDocumentIds(QVariant &obj, const QVariantList &docIds);

    /** Returns a Apple Wallet pass identifier if present in the documents ids of @p obj. */
    KITINERARY_EXPORT QUrl pkPassId(const QVariant &obj);
}

}

