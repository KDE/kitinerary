/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_GENERATOR_COUNTRYDBGENERATOR_H
#define KITINERARY_GENERATOR_COUNTRYDBGENERATOR_H

#include <QSet>
#include <QUrl>

#include <map>
#include <vector>

class QIODevice;
class QJsonObject;

namespace KItinerary {
namespace Generator {

/** Generate country data tables. */
class CountryDbGenerator
{
public:
    bool generate(QIODevice *out);

    struct Country
    {
        QUrl uri;
        QString name;
        QString isoCode;
        QString drivingSide;
        QSet<QString> powerPlugTypes;
    };

private:
    bool fetchCountryList();
    bool fetchDrivingDirections();
    bool fetchPowerPlugTypes();
    bool fetchUicCountryCodes();
    QUrl insertOrMerge(const QJsonObject &obj);
    void writeCountryTable(QIODevice *out);
    void writeIso3CodeTable(QIODevice *out);
    void writeUicCodeTable(QIODevice *out);
    void printSummary();

    std::vector<Country> m_countries;
    std::map<QString, QUrl> m_isoCodeMap;
    std::map<uint16_t, QString> m_uicCodeMap;
    std::map<QString, QString> m_iso3CodeMap;

    int m_isoCodeConflicts = 0;
};

}}

#endif // KITINERARY_GENERATOR_COUNTRYDBGENERATOR_H
