/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

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
    void writeUicCodeTable(QIODevice *out);
    void printSummary();

    std::vector<Country> m_countries;
    std::map<QString, QUrl> m_isoCodeMap;
    std::map<uint16_t, QString> m_uicCodeMap;

    int m_isoCodeConflicts = 0;
};

}}

