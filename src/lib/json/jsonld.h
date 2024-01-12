/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_JSONLD_H
#define KITINERARY_JSONLD_H

class QJsonObject;
class QString;
class QStringView;

namespace KItinerary {

/** Utility methods for working with schema.org data in JSON-LD format. */
namespace JsonLd
{
    /** Normalized type name from @p object. */
    [[nodiscard]] QString typeName(const QJsonObject &obj);
    [[nodiscard]] QString normalizeTypeName(QString &&typeName);

    /** Checks whether @p uri is in the http://schema.org namespace.
     *  That is more complex than just startsWith since schema.org started
     *  to use "https" in their URIs...
     */
    [[nodiscard]] bool isSchemaOrgNamespace(QStringView uri);
};

}

#endif // KITINERARY_JSONLD_H
