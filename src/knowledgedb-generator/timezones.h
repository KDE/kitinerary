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

#include <QImage>
#include <QHash>

class QByteArray;
class QColor;

namespace KItinerary {
namespace Generator {

class Timezones
{
public:
    Timezones();
    ~Timezones();

    QByteArray timezoneForLocation(const QString &isoCode, const KnowledgeDb::Coordinate &coord) const;

    // the offset into the timezone string table
    uint16_t offset(const QByteArray &tz) const;

private:
    friend class TimezoneDbGenerator;
    QPoint coordinateToPixel(const KnowledgeDb::Coordinate &coord) const;
    QByteArray timezoneForPixel(int x, int y) const;

    mutable QImage m_map;
    QHash<QRgb, QByteArray> m_colorMap;
    double m_xMapUnitsPerPixel;
    double m_yMapUnitsPerPixel;
    double m_topLeftMapUnitX;
    double m_topLeftMapUnitY;

    std::vector<QByteArray> m_zones;
    std::vector<uint16_t> m_zoneOffsets;

    std::map<QString, std::vector<QByteArray>> m_countryZones;
};

}
}

#endif
