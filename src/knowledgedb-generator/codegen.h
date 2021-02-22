/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_GENERATOR_CODEGEN_H
#define KITINERARY_GENERATOR_CODEGEN_H

#include "knowledgedb.h"

class QByteArray;
class QIODevice;
class QString;

namespace KItinerary {
namespace Generator {


/** Code generation utilities. */
namespace CodeGen
{
    void writeLicenseHeaderWikidata(QIODevice *out);
    void writeLicenseHeaderOSM(QIODevice *out);
    void writeCoordinate(QIODevice *out, KnowledgeDb::Coordinate coord);
    void writeCountryIsoCode(QIODevice *out, const QString &isoCode);
    void writeTimezone(QIODevice *out, const QByteArray &tzName);
    void writeTimezoneEnum(QIODevice *out, const QByteArray &tzName);
}

}
}

#endif // KITINERARY_GENERATOR_CODEGEN_H
