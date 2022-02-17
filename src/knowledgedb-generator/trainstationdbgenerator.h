/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <knowledgedb.h>
#include <stationidentifier.h>
#include <iatacode.h>

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
    template <typename Id>
    bool fetch(const char *prop, const char *name, std::map<Id, QUrl> &idMap);
    bool fetchIndianRailwaysStationCode();
    bool fetchFinishStationCodes();
    bool fetchCountryInformation();
    QUrl insertOrMerge(const QJsonObject &obj, bool mergeOnly = false);
    void processStations();
    void writeStationData(QIODevice *out);
    template <typename Id>
    void writeIdMap(QIODevice *out, const std::map<Id, QUrl> &idMap, const char *tabName, const char *typeName) const;
    void writeIndianRailwaysMap(QIODevice *out);
    void writeVRMap(QIODevice *out);
    void printSummary();

    template <typename Id>
    QByteArray encodeId(Id id) const { return '"' + id.toString().toUtf8() + '"'; }
    QByteArray encodeId(KnowledgeDb::IBNR id) const { return QByteArray::number(id.value()); }
    QByteArray encodeId(KnowledgeDb::UICStation id) const { return QByteArray::number(id.value()); }

    std::vector<Station> m_stations;
    std::map<KnowledgeDb::IBNR, QUrl> m_ibnrMap;
    std::map<KnowledgeDb::UICStation, QUrl> m_uicMap;
    std::map<KnowledgeDb::SncfStationId, QUrl> m_sncfIdMap;
    std::map<KnowledgeDb::BenerailStationId, QUrl> m_benerailIdMap;
    std::map<QString, QUrl> m_indianRailwaysMap;
    std::map<KnowledgeDb::VRStationCode, QUrl> m_vrfiMap;
    std::map<KnowledgeDb::IataCode, QUrl> m_iataMap;
    std::map<KnowledgeDb::AmtrakStationCode, QUrl> m_amtrakMap;

    int m_idConflicts = 0;
    int m_idFormatViolations = 0;
    int m_timezoneLookupFailure = 0;
    int m_coordinateConflicts = 0;
    int m_countryConflicts = 0;
};

}
}

