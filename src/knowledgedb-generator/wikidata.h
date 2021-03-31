/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

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
    QJsonArray query(const QString &sparqlQuery, const QString &cacheFileName);
}

}
}

