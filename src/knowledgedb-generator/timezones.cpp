/*
   SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "timezones.h"

#include <QDebug>
#include <QFile>

#include <cassert>
#include <limits>

using namespace KItinerary::Generator;

Timezones::Timezones()
{
    // load zone.tab for country mapping
    QFile zoneTab(QStringLiteral("/usr/share/zoneinfo/zone.tab"));
    if (!zoneTab.open(QFile::ReadOnly)) {
        qCritical() << "Unable to open zonetab file: " << zoneTab.errorString();
        exit(1);
    }

    const auto lines = QString::fromUtf8(zoneTab.readAll()).split(QLatin1Char('\n'));
    for (const auto &line : lines) {
        if (line.startsWith(QLatin1Char('#'))) {
            continue;
        }

        const auto cols = line.split(QLatin1Char('\t'));
        if (cols.size() < 3) {
            continue;
        }

        const auto countries = cols.at(0).split(QLatin1Char(','));
        const auto tzName = cols.at(2).toUtf8();

        m_zones.push_back(tzName);
        for (const auto &country : countries) {
            m_countryZones[country].push_back(tzName);
        }
        if (countries.size() == 1) {
            if (m_countryForZone.find(tzName) != m_countryForZone.end()) {
                m_countryForZone[tzName] = QString();
            } else {
                m_countryForZone[tzName] = countries[0];
            }
        }
    }

    /* Remove non-official zones that openSUSE patches into their zonetab. */
    removeZone("Asia/Beijing");

    /* Manual overrides for countries that de-facto only have a single timezone,
     * even if the IANA database doesn't reflect that.
    */
    m_countryZones[QStringLiteral("AR")] = { "America/Argentina/Buenos_Aires" };
    m_countryZones[QStringLiteral("CN")] = { "Asia/Shanghai" };
    m_countryZones[QStringLiteral("CY")] = { "Asia/Nicosia" };
    m_countryZones[QStringLiteral("DE")] = { "Europe/Berlin" };
    m_countryZones[QStringLiteral("MY")] = { "Asia/Kuala_Lumpur" };

    /* Manual overrides for timezones that do not belong to a unique country, contrary what zonetab claims. */
    setCountryForZone("Asia/Bangkok", {}); // also used in northern Vietnam
    setCountryForZone("Europe/Simferopol", {}); // disputed area

    // create offset index
    std::sort(m_zones.begin(), m_zones.end());
    m_zoneOffsets.reserve(m_zones.size());
    uint16_t offset = 0;
    for (const auto &tz : m_zones) {
        m_zoneOffsets.push_back(offset);
        offset += tz.size() + 1; // +1 of the trailing null byte
    }

}

Timezones::~Timezones() = default;

uint16_t Timezones::offset(const QByteArray& tz) const
{
    const auto it = std::lower_bound(m_zones.begin(), m_zones.end(), tz);
    if (it == m_zones.end() || (*it) != tz) {
        return std::numeric_limits<uint16_t>::max();
    }
    return m_zoneOffsets[std::distance(m_zones.begin(), it)];
}

void Timezones::setCountryForZone(const QByteArray &tz, const QString &country)
{
    auto it = m_countryForZone.find(tz);
    if (it == m_countryForZone.end()) {
        return;
    }
    (*it).second = country;
}

void Timezones::removeZone(const QByteArray &tz)
{
    m_zones.erase(std::remove(m_zones.begin(), m_zones.end(), tz), m_zones.end());
    m_countryForZone.erase(tz);
    for (auto &it : m_countryZones) {
        it.second.erase(std::remove(it.second.begin(), it.second.end(), tz), it.second.end());
    }
}
