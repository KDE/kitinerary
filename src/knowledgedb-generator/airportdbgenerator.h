/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "knowledgedb.h"
#include "osmairportdb.h"
#include "timezones.h"

#include <QHash>
#include <QMap>
#include <QUrl>

class QIODevice;

namespace KItinerary {
namespace Generator {

/** Generate airport database from Wikidata. */
class AirportDbGenerator
{
public:
    bool generate(QIODevice *out);

    struct Airport
    {
        QUrl uri;
        QString iataCode;
        QString icaoCode;
        QString label;
        QString alias;
        QString country;
        QByteArray tz;
        QList<QString> fragments; // name string fragments used for indexing
        int tzOffset;
        KnowledgeDb::Coordinate coord;
    };

    OSMAirportDb osmDb;

private:
    bool fetchAirports();
    bool fetchCountries();
    void merge(Airport &lhs, const Airport &rhs);
    void improveCoordinates();
    void indexNames();

    QHash<QUrl, Airport> m_airportMap;
    QMap<QString, QUrl> m_iataMap;
    // mapping IATA codes to indexed string fragments
    QMap<QString, QList<QString>> m_labelMap;

    int m_iataCollisions = 0;
    int m_coordinateConflicts = 0;
    int m_countryConflicts = 0;
    int m_timezoneLoopupFails = 0;
};

}
}

