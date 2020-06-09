/*
   Copyright (c) 2017 Volker Krause <vkrause@kde.org>

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   This library is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
   License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.
*/

#ifndef KITINERARY_GENERATOR_TIMEZONES
#define KITINERARY_GENERATOR_TIMEZONES

#include <knowledgedb.h>

#include <QByteArray>
#include <QString>

#include <map>
#include <vector>

namespace KItinerary {
namespace Generator {

class Timezones
{
public:
    Timezones();
    ~Timezones();

    QByteArray timezoneForLocation(const QString &isoCode, KnowledgeDb::Coordinate coord) const;

    // the offset into the timezone string table
    uint16_t offset(const QByteArray &tz) const;

private:
    friend class TimezoneDbGenerator;

    void setCountryForZone(const QByteArray &tz, const QString &country);

    std::vector<QByteArray> m_zones;
    std::vector<uint16_t> m_zoneOffsets;

    std::map<QString, std::vector<QByteArray>> m_countryZones;
    std::map<QByteArray, QString> m_countryForZone;
};

}
}

#endif
