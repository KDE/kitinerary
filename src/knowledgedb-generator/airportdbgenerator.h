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
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KITINERARY_AIRPORTDBGENERATOR_H
#define KITINERARY_AIRPORTDBGENERATOR_H

#include <knowledgedb.h>
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
        QByteArray tz;
        int tzOffset;
        KnowledgeDb::Coordinate coord;
    };
private:
    bool fetchAirports();
    void merge(Airport &lhs, const Airport &rhs);
    void lookupTimezones();
    void indexNames();

    QHash<QUrl, Airport> m_airportMap;
    QMap<QString, QUrl> m_iataMap;
    QMap<QString, QVector<QString>> m_labelMap;

    Timezones m_tzDb;

    int m_iataCollisions = 0;
    int m_coordinateConflicts = 0;
    int m_timezoneLoopupFails = 0;
};

}
}

#endif // KITINERARY_AIRPORTDBGENERATOR_H
