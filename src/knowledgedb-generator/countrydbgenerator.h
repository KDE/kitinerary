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
        QString drivingSide;
        QSet<QString> powerPlugTypes;
    };

private:
    bool fetchCountryList();
    bool fetchDrivingDirections();
    bool fetchPowerPlugTypes();
    QUrl insertOrMerge(const QJsonObject &obj);
    void writeCountryTable(QIODevice *out);
    void printSummary();

    std::vector<Country> m_countries;
    std::map<QString, QUrl> m_isoCodeMap;

    int m_isoCodeConflicts = 0;
};

}}

#endif // KITINERARY_GENERATOR_COUNTRYDBGENERATOR_H
