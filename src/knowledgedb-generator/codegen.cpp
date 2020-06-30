/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "codegen.h"
#include "timezones.h"

#include <QIODevice>

using namespace KItinerary::Generator;

void CodeGen::writeLicenseHeader(QIODevice *out)
{
    out->write(R"(/*
 * SPDX-License-Identifier: ODbL-1.0
 *
 * This code is auto-generated from OpenStreetMap (licensed under ODbL) and Wikidata (licensed under CC0), do not edit!
 */
)");
}

void CodeGen::writeCoordinate(QIODevice* out, KnowledgeDb::Coordinate coord)
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
        out->write("Tz::Undefined");
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
