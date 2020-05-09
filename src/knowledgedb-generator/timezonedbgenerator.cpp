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

#include "timezonedbgenerator.h"
#include "codegen.h"
#include "timezones.h"

#include <QDebug>
#include <QIODevice>

using namespace KItinerary::Generator;

void TimezoneDbGenerator::generate(QIODevice *out)
{
    CodeGen::writeLicenseHeader(out);

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

}
}
)");
}

void TimezoneDbGenerator::generateHeader(QIODevice *out)
{
    CodeGen::writeLicenseHeader(out);

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
