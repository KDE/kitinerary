/*
   SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
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

    // the offset into the timezone string table
    uint16_t offset(const QByteArray &tz) const;

private:
    friend class TimezoneDbGenerator;

    void setCountryForZone(const QByteArray &tz, const QString &country);
    void removeZone(const QByteArray &tz);

    std::vector<QByteArray> m_zones;
    std::vector<uint16_t> m_zoneOffsets;

    std::map<QString, std::vector<QByteArray>> m_countryZones;
    std::map<QByteArray, QString> m_countryForZone;
};

}
}

#endif
