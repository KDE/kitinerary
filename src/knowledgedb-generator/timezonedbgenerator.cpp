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
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
namespace KItinerary {
namespace KnowledgeDb {

// timezone name strings
static const char timezone_names[] =
)");
    // timezone string tables
    for (const auto &tz : tzDb.m_zones) {
        out->write("    ");
        out->write("\"");
        out->write(tz);
        out->write("\\0\"\n");
    }
    out->write(R"(;

}
}
)");
}
