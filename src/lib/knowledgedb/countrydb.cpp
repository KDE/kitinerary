/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "config-kitinerary.h"
#include "countrydb.h"
#include "countrydb_data.cpp"

#include <QDebug>
#include <QString>

#include <algorithm>

using namespace KItinerary;
using namespace KItinerary::KnowledgeDb;

static_assert(sizeof(CountryId) <= 2, "CountryId too large");

Country KnowledgeDb::countryForId(CountryId id)
{
    const auto it = std::lower_bound(std::begin(country_table), std::end(country_table), id, [](const Country &lhs, CountryId rhs) {
        return lhs.id < rhs;
    });
    if (it == std::end(country_table) || (*it).id != id) {
        return {CountryId{}, DrivingSide::Unknown, Unknown};
    }
    return (*it);
}

const Country* KnowledgeDb::countriesBegin()
{
    return std::begin(country_table);
}

const Country* KnowledgeDb::countriesEnd()
{
    return std::end(country_table);
}

struct PowerPlugCompatMap {
    PowerPlugType plug;
    PowerPlugTypes sockets;
};

static const PowerPlugCompatMap power_plug_compat_map[] = {
    { TypeA, TypeA | TypeB },
    { TypeB, TypeB },
    { TypeC, TypeC | TypeE | TypeF | TypeH | TypeJ | TypeK | TypeL | TypeN },
    { TypeD, TypeD },
    { TypeE, TypeE | TypeF | TypeK }, // TypeE <-> TypeF not strictly correct, but in practice almost always compatible
    { TypeF, TypeE | TypeF | TypeK },
    { TypeG, TypeG },
    { TypeH, TypeH },
    { TypeI, TypeI },
    { TypeJ, TypeJ },
    { TypeK, TypeK },
    { TypeL, TypeL },
    { TypeM, TypeM },
    { TypeN, TypeN }
};

PowerPlugTypes KnowledgeDb::incompatiblePowerPlugs(PowerPlugTypes plugs, PowerPlugTypes sockets)
{
    PowerPlugTypes failPlugs{};
    for (const auto map : power_plug_compat_map) {
        if ((plugs & map.plug) == 0) {
            continue;
        }
        if ((map.sockets & sockets) == 0) {
            failPlugs |= map.plug;
        }
    }
    return failPlugs;
}

PowerPlugTypes KnowledgeDb::incompatiblePowerSockets(PowerPlugTypes plugs, PowerPlugTypes sockets)
{
    PowerPlugTypes failSockets{};
    for (const auto map : power_plug_compat_map) {
        if ((plugs & map.plug) == 0) {
            continue;
        }
        if ((map.sockets & ~sockets) != 0) {
            failSockets |= (map.sockets ^ sockets) & sockets;
        }
    }
    return failSockets & ~plugs;
}

#if !HAVE_KI18N_LOCALE_DATA
KnowledgeDb::CountryId KnowledgeDb::countryIdFromIso3166_1alpha3(CountryId3 iso3Code)
{
    const auto it = std::lower_bound(std::begin(iso_country_code_table), std::end(iso_country_code_table), iso3Code, [](const auto &lhs, CountryId3 rhs) {
        return lhs.iso3Code < rhs;
    });
    if (it == std::end(iso_country_code_table) || (*it).iso3Code != iso3Code) {
        return {};
    }

    return (*it).iso2Code;
}
#endif

KnowledgeDb::CountryId KnowledgeDb::countryIdForUicCode(uint16_t uicCountryCode)
{
    const auto it = std::lower_bound(std::begin(uic_country_code_table), std::end(uic_country_code_table), uicCountryCode, [](const auto &lhs, uint16_t rhs) {
        return lhs.uicCode < rhs;
    });
    if (it == std::end(uic_country_code_table) || (*it).uicCode != uicCountryCode) {
        return {};
    }

    return (*it).isoCode;
}

#include "moc_countrydb.cpp"
