/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "timezonedbgenerator.h"
#include "codegen.h"
#include "timezones.h"

#include <QDebug>
#include <QIODevice>

using namespace KItinerary::Generator;

void TimezoneDbGenerator::generate(QIODevice *out)
{
    CodeGen::writeLicenseHeaderOSM(out);

    Timezones tzDb;

    out->write(R"(
#include "timezonedb_p.h"
#include "timezonedb_data.h"

namespace KItinerary {
namespace KnowledgeDb {

// timezone name strings
static const char timezone_names[] =
)");
    // timezone string table
    for (const auto &tz : tzDb.m_zones) {
        out->write("    ");
        out->write("\"");
        out->write(tz);
        out->write("\\0\"\n");
    }
    out->write(R"(;

static constexpr const uint16_t timezone_names_offsets[] = {
    )");
    out->write(QByteArray::number(tzDb.m_zones.front().size()));
    out->write(", // Undefined\n");

    // offsets into timezone string table
    for (const auto &tz : tzDb.m_zones) {
        out->write("    ");
        out->write(QByteArray::number(tzDb.offset(tz)));
        out->write(", // ");
        out->write(tz);
        out->write("\n");
    }

    out->write(R"(};

static constexpr const CountryTimezoneMap country_timezone_map[] = {
)");

    for (const auto &map : tzDb.m_countryZones) {
        if (map.second.size() != 1) {
            continue;
        }
        out->write("    { ");
        CodeGen::writeCountryIsoCode(out, map.first);
        out->write(", ");
        CodeGen::writeTimezone(out, map.second.at(0));
        out->write(" },\n");
    }

    out->write(R"(};

static constexpr const CountryId timezone_country_map[] = {
    CountryId{}, // Undefined
)");

    for (const auto &tz : tzDb.m_zones) {
        out->write("    ");
        CodeGen::writeCountryIsoCode(out, tzDb.m_countryForZone[tz]);
        out->write(", // ");
        out->write(tz);
        out->write("\n");
    }

    out->write(R"(};

}
}
)");
}

void TimezoneDbGenerator::generateHeader(QIODevice *out)
{
    CodeGen::writeLicenseHeaderOSM(out);

    Timezones tzDb;
    out->write(R"(
#ifndef KITINERARY_KNOWLEDGEDB_TIMEZONEDB_DATA_H
#define KITINERARY_KNOWLEDGEDB_TIMEZONEDB_DATA_H

#include <cstdint>

namespace KItinerary {
namespace KnowledgeDb {

/** Enum representing all timezones. */
enum class Tz : uint16_t {
    Undefined,
)");

    for (const auto &tz : tzDb.m_zones) {
        out->write("    ");
        CodeGen::writeTimezoneEnum(out, tz);
        out->write(",\n");
    }
    out->write(R"(};

}
}

#endif
)");
}
