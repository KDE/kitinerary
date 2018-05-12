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

#ifndef KITINERARY_GENERATOR_CODEGEN_H
#define KITINERARY_GENERATOR_CODEGEN_H

#include "knowledgedb.h"

class QByteArray;
class QIODevice;

namespace KItinerary {
namespace Generator {

class Timezones;

/** Code generation utilities. */
namespace CodeGen
{
    void writeLicenseHeader(QIODevice *out);
    void writeCoordinate(QIODevice *out, const KnowledgeDb::Coordinate &coord);
    void writeTimezone(QIODevice *out, Timezones *tzDb, const QByteArray &tzName);
}

}
}

#endif // KITINERARY_GENERATOR_CODEGEN_H
