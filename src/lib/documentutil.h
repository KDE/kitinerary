/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

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

