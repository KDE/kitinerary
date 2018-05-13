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
        m_colorMap.insert(c.rgb(), split.at(4).trimmed());
    }

    std::sort(m_zones.begin(), m_zones.end());
    m_zoneOffsets.reserve(m_zones.size());
    uint16_t offset = 0;
    for (const auto &tz : m_zones) {
        m_zoneOffsets.push_back(offset);
        offset += tz.size() + 1; // +1 of the trailing null byte
    }
}

Timezones::~Timezones() = default;

QByteArray Timezones::timezoneForCoordinate(const KnowledgeDb::Coordinate &coord) const
{
    if (!coord.isValid()) {
        return {};
    }

    if (m_map.isNull() && !m_map.load(QStringLiteral("timezones.png"))) {
        qCritical() << "Unable to open timezone map.";
        exit(1);
    }

    const int x = qRound(m_map.width() * ((coord.longitude + 180.0f)/ 360.0f));
    const int y = qRound(-m_map.height() * ((coord.latitude - 90.0f) / 180.0f));

    //qDebug() << x << y << m_map.width() << m_map.height() << longitude << latitude << QColor(m_map.pixel(x, y)) << m_zones.value(m_map.pixel(x, y));
    const auto tz = timezoneForPixel(x, y);
    if (!tz.isEmpty()) {
        return m_colorMap.value(m_map.pixel(x, y));
    }

    // search the vicinity, helps with costal/island airports
    const struct {
        int x;
        int y;
    } offsets[] = { {-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1} };
    QSet<QByteArray> tzs;
    for (int i = 0; i < 8; ++i) {
        const auto tz = timezoneForPixel(x + offsets[i].x, y + offsets[i].y);
        if (!tz.isEmpty()) {
            tzs.insert(tz);
        }
    }
    if (tzs.size() == 1) {
        return *tzs.constBegin();
    }
    //qDebug() << "Timezone candidates:" << tzs;
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
