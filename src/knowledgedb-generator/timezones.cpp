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

    // load zone.tab for country mapping
    QFile zoneTab(QStringLiteral("/usr/share/zoneinfo/zone1970.tab"));
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
    QSet<QByteArray> coordTzs;
    if (coord.isValid()) {
        if (m_map.isNull() && !m_map.load(QStringLiteral("timezones.png"))) {
            qCritical() << "Unable to open timezone map.";
            exit(1);
        }

        const int x = qRound(m_map.width() * ((coord.longitude + 180.0f)/ 360.0f));
        const int y = qRound(-m_map.height() * ((coord.latitude - 90.0f) / 180.0f));

        //qDebug() << x << y << m_map.width() << m_map.height() << longitude << latitude << QColor(m_map.pixel(x, y)) << m_zones.value(m_map.pixel(x, y));
        const auto tz = timezoneForPixel(x, y);
        if (!tz.isEmpty()) {
            coordTzs.insert(tz);
        }

        // search the vicinity, helps with costal/island airports
        if (coordTzs.isEmpty()) {
            const struct {
                int x;
                int y;
            } offsets[] = { {-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1} };
            for (int i = 0; i < 8; ++i) {
                const auto tz = timezoneForPixel(x + offsets[i].x, y + offsets[i].y);
                if (!tz.isEmpty()) {
                    coordTzs.insert(tz);
                }
            }
        }
    }

    // determine the result: either just one method yieled a unique result, or both have a unique intersection
    if (countryTzs.size() == 1) {
        return *countryTzs.constBegin();
    } else if (coordTzs.size() == 1) {
        return *coordTzs.constBegin();
    }
    //qDebug() << "Timezone candidates:" << isoCode << coordTzs << countryTzs << coord.latitude << coord.longitude;

    const auto tzs = coordTzs.intersect(countryTzs);
    if (tzs.size() == 1) {
        return *tzs.constBegin();
    }
    return {};
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
