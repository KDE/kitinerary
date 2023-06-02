/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_JSONLD_H
#define KITINERARY_JSONLD_H

class QJsonObject;
class QString;

namespace KItinerary {

/** Utility methods for working with schema.org data in JSON-LD format. */
namespace JsonLd
{
    /** Normalized type name from @p object. */
    QString typeName(const QJsonObject &obj);
};

}

#endif // KITINERARY_JSONLD_H
