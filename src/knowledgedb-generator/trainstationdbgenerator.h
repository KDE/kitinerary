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

#ifndef KITINERARY_GENERATOR_TRAINSTATIONDBGENERATOR_H
#define KITINERARY_GENERATOR_TRAINSTATIONDBGENERATOR_H

#include <knowledgedb.h>

#include <QByteArray>
#include <QString>
#include <QUrl>

#include <map>
#include <vector>

class QIODevice;
class QJsonObject;

namespace KItinerary {
namespace Generator {

/** Generate train station data tables. */
class TrainStationDbGenerator
{
public:
    bool generate(QIODevice *out);

    struct Station
    {
        QUrl uri;
        QString name;
        KnowledgeDb::Coordinate coord;
        QString isoCode;
    };

private:
    bool fetchIBNR();
    bool fetchUIC();
    bool fetchSncf();
    bool fetchIndianRailwaysStationCode();
    bool fetchFinishStationCodes();
    bool fetchCountryInformation();
    QUrl insertOrMerge(const QJsonObject &obj, bool mergeOnly = false);
    void processStations();
    void writeStationData(QIODevice *out);
    void writeIBNRMap(QIODevice *out);
    void writeUICMap(QIODevice *out);
    void writeSncfMap(QIODevice *out);
    void writeIndianRailwaysMap(QIODevice *out);
    void writeVRMap(QIODevice *out);
    void printSummary();

    std::vector<Station> m_stations;
    std::map<uint32_t, QUrl> m_ibnrMap;
    std::map<uint32_t, QUrl> m_uicMap;
    std::map<QString, QUrl> m_sncfIdMap;
    std::map<QString, QUrl> m_indianRailwaysMap;
    std::map<QString, QUrl> m_vrfiMap;

    int m_idConflicts = 0;
    int m_idFormatViolations = 0;
    int m_timezoneLookupFailure = 0;
    int m_coordinateConflicts = 0;
    int m_countryConflicts = 0;
};

}
}

#endif // KITINERARY_GENERATOR_TRAINSTATIONDBGENERATOR_H
