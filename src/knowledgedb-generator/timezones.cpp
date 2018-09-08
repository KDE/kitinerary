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

#include "timezones.h"

#include <QDebug>
#include <QFile>

#include <cassert>
#include <limits>

using namespace KItinerary::Generator;

Timezones::Timezones()
{
    // load the color to timezone mapping file
    QFile colorMap(QStringLiteral("timezones.colormap"));
    if (!colorMap.open(QFile::ReadOnly)) {
        qCritical() << "Unable to open timezone colormap file: " << colorMap.errorString();
        exit(1);
    }

    while (!colorMap.atEnd()) {
        const auto line = colorMap.readLine();
        const auto split = line.split(',');
        if (split.size() < 5) {
            continue;
        }

        const auto tzName = split.at(4).trimmed();
        if (tzName.isEmpty()) {
            continue;
        }
        m_zones.push_back(tzName);

        QColor c;
        c.setRed(split.at(0).toInt());
        c.setGreen(split.at(1).toInt());
        c.setBlue(split.at(2).toInt());
        if (m_colorMap.contains(c.rgb())) {
            qWarning() << "Color collision on timezones:" << m_colorMap.value(c.rgb()) << split.at(4).trimmed();
        }
        m_colorMap.insert(c.rgb(), split.at(4).trimmed());
    }

    std::sort(m_zones.begin(), m_zones.end());
    m_zoneOffsets.reserve(m_zones.size());
    uint16_t offset = 0;
    for (const auto &tz : m_zones) {
        m_zoneOffsets.push_back(offset);
        offset += tz.size() + 1; // +1 of the trailing null byte
    }

    // load the wold file for correcting the pixel to coordinate mapping in the timezone image
    // see https://en.wikipedia.org/wiki/World_file for format and math behind this
    QFile worldFile(QStringLiteral("timezones.pgw"));
    if (!worldFile.open(QFile::ReadOnly|QFile::Text)) {
        qCritical() << "Unable to open world file: " << worldFile.errorString();
        exit(1);
    }
    const auto worldFileContent = worldFile.readAll().split('\n');
    if (worldFileContent.size() < 6) {
        qCritical() << "Invalid world map file format.";
        exit(1);
    }
    if (worldFileContent[1].toDouble() != 0.0 || worldFileContent[2].toDouble() != 0.0) {
        qCritical() << "Timezone map is rotated, that is not supported!";
        exit(1);
    }
    m_xMapUnitsPerPixel = worldFileContent[0].toDouble();
    m_yMapUnitsPerPixel = worldFileContent[3].toDouble();
    m_topLeftMapUnitX = worldFileContent[4].toDouble();
    m_topLeftMapUnitY = worldFileContent[5].toDouble();

    // load zone.tab for country mapping
    QFile zoneTab(QStringLiteral("/usr/share/zoneinfo/zone.tab"));
    if (!zoneTab.open(QFile::ReadOnly)) {
        qCritical() << "Unable to open zonetab file: " << zoneTab.errorString();
        exit(1);
    }

    for (const auto &line : QString::fromUtf8(zoneTab.readAll()).split(QLatin1Char('\n'))) {
        if (line.startsWith(QLatin1Char('#'))) {
            continue;
        }

        const auto cols = line.split(QLatin1Char('\t'));
        if (cols.size() < 3) {
            continue;
        }

        const auto countries = cols.at(0).split(QLatin1Char(','));
        for (const auto &country : countries) {
            m_countryZones[country].push_back(cols.at(2).toUtf8());
        }
    }

    /* Manual overrides for countries that de-facto only have a single timezone,
     * even if the IANA database doesn't reflect that.
    */
    m_countryZones[QStringLiteral("CN")] = { "Asia/Shanghai" };
    m_countryZones[QStringLiteral("CY")] = { "Asia/Nicosia" };
    m_countryZones[QStringLiteral("DE")] = { "Europe/Berlin" };
    m_countryZones[QStringLiteral("MY")] = { "Asia/Kuala_Lumpur" };
}

Timezones::~Timezones() = default;

QByteArray Timezones::timezoneForLocation(const QString &isoCode, const KnowledgeDb::Coordinate &coord) const
{
    // look up by country
    QSet<QByteArray> countryTzs;
    const auto it = m_countryZones.find(isoCode);
    if (it != m_countryZones.end()) {
        for (const auto &tz : (*it).second) {
            countryTzs.insert(tz);
        }
    }

    // look up by coordinate
    QByteArray coordTz; // search radius 0
    QSet<QByteArray> coordTzs; // search radius 1
    if (coord.isValid()) {
        if (m_map.isNull() && !m_map.load(QStringLiteral("timezones.png"))) {
            qCritical() << "Unable to open timezone map.";
            exit(1);
        }

        const auto p = coordinateToPixel(coord);
        //qDebug() << p.x() << p.y() << m_map.width() << m_map.height() << coord.longitude << coord.latitude << QColor(m_map.pixel(p)) << m_colorMap.value(m_map.pixel(p));
        coordTz = timezoneForPixel(p.x(), p.y());

        // search the vicinity, helps with islands/costal regions or border regions
        const QPoint offsets[] = { {-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1} };
        for (auto offset : offsets) {
            const auto tz = timezoneForPixel(p.x() + offset.x(), p.y() + offset.y());
            if (!tz.isEmpty()) {
                coordTzs.insert(tz);
            }
        }
    }

    // determine the result: either just one method yieled a unique result, or both have a unique intersection
    if (coordTz.isEmpty() && countryTzs.size() == 1) {
        return *countryTzs.constBegin();
    }
    if (!coordTz.isEmpty() && (countryTzs.isEmpty() || countryTzs.contains(coordTz))) {
        return coordTz;
    }

    // if the above wasn't the case, look for a unique intersection in the vicinity of the coordinate
    // this covers cases of locations within the 1.5km resolution of the timezone image
    coordTzs.insert(coordTz);
    const auto tzs = coordTzs.intersect(countryTzs);
    if (tzs.size() == 1) {
        return *tzs.constBegin();
    }

    // if the above still doesn't help, we take the coodinate-based result, can't be entirely wrong
    //qDebug() << "Timezone candidates:" << isoCode << coordTz << coordTzs << countryTzs << coord.latitude << coord.longitude;
    return coordTz;
}

QByteArray Timezones::timezoneForPixel(int x, int y) const
{
    x %= m_map.width();
    y %= m_map.height();
    return m_colorMap.value(m_map.pixel(x, y));
}

uint16_t Timezones::offset(const QByteArray& tz) const
{
    const auto it = std::lower_bound(m_zones.begin(), m_zones.end(), tz);
    if (it == m_zones.end() || (*it) != tz) {
        return std::numeric_limits<uint16_t>::max();
    }
    return m_zoneOffsets[std::distance(m_zones.begin(), it)];
}

QPoint Timezones::coordinateToPixel(const KnowledgeDb::Coordinate &coord) const
{
    QPoint p;
    p.setX(qRound((coord.longitude - m_topLeftMapUnitX) / m_xMapUnitsPerPixel));
    p.setY(qRound((coord.latitude - m_topLeftMapUnitY) / m_yMapUnitsPerPixel));
    return p;
}
