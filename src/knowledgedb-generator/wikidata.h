/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

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

#ifndef KITINERARY_WIKIDATA_H
#define KITINERARY_WIKIDATA_H

#include "knowledgedb.h"

class QJsonArray;
class QString;

namespace KItinerary {
namespace Generator {

/** Utilities for interaction with Wikidata. */
namespace WikiData
{
    /** Parse a geo coordinate from a Wikidata JSON-LD response. */
    KnowledgeDb::Coordinate parseCoordinate(const QString &value);

    /** Retrieve the result of a SPARQL query from Wikidata.
     *  This has the ability to use an already retrieved local cache
     *  instead, for reducing query load and bandwidth use while working
     *  on the generator code.
     *  @warning This uses the QCoreApplication event loop to emulated
     *  synchronous calls, do never ever use this outside of a simple
     *  CLI tool!
     */
    QJsonArray query(const char *sparqlQuery, const char *cacheFileName);
}

}
}

#endif // KITINERARY_WIKIDATA_H
