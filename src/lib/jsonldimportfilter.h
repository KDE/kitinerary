/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

class QJsonArray;
class QJsonObject;

namespace KItinerary {

/** Filter input JSON for loading with JsonLdDocument, to deal with obsolete
 *  or renamed elements.
 */
namespace JsonLdImportFilter
{
    /** Filter the top-level object @p obj for loading with JsonLdDocument.
     *  Due to type and graph expansion, the result can actually contain multiple object.
     */
    QJsonArray filterObject(const QJsonObject &obj);
}

}

