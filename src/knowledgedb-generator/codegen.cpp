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

#include "codegen.h"
#include "timezones.h"

#include <QIODevice>

using namespace KItinerary::Generator;

void CodeGen::writeLicenseHeader(QIODevice *out)
{
    out->write(R"(/*
 * This code is auto-generated from Wikidata data. Licensed under CC0.
 */
)");
}

void CodeGen::writeCoordinate(QIODevice* out, const KnowledgeDb::Coordinate& coord)
{
    out->write("Coordinate{");
    if (coord.isValid()) {
        out->write(QByteArray::number(coord.longitude));
        out->write(", ");
        out->write(QByteArray::number(coord.latitude));
    }
    out->write("}");
}

void CodeGen::writeCountryIsoCode(QIODevice *out, const QString &isoCode)
{
    out->write("CountryId{");
    if (!isoCode.isEmpty()) {
        out->write("\"");
        out->write(isoCode.toUtf8());
        out->write("\"");
    }
    out->write("}");
}

void CodeGen::writeTimezone(QIODevice *out, const QByteArray &tzName)
{
    if (tzName.isEmpty()) {
        out->write("Timezone{}");
    } else {
        out->write("Tz::");
        writeTimezoneEnum(out, tzName);
    }
}

void CodeGen::writeTimezoneEnum(QIODevice* out, const QByteArray& tzName)
{
    auto enumName(tzName);
    out->write(enumName.replace("/", "_").replace("-", "_"));
}
