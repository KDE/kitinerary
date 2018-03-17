/*
   Copyright (c) 2017 Volker Krause <vkrause@kde.org>

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   This library is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
   License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.
*/

#ifndef JSONLDDOCUMENT_H
#define JSONLDDOCUMENT_H

#include "kitinerary_export.h"

#include <QVariant>
#include <QVector>

class QJsonArray;

/** Serialization/deserialization code for JSON-LD data. */
namespace JsonLdDocument {
KITINERARY_EXPORT QVector<QVariant> fromJson(const QJsonArray &array);
KITINERARY_EXPORT QJsonArray toJson(const QVector<QVariant> &data);

/** Read property @p name on object @p obj. */
KITINERARY_EXPORT QVariant readProperty(const QVariant &obj, const char *name);
/** Set property @p name on object @p obj to value @p value. */
KITINERARY_EXPORT void writeProperty(QVariant &obj, const char *name, const QVariant &value);
/** Removes property @p name on object @p obj. */
KITINERARY_EXPORT void removeProperty(QVariant &obj, const char *name);
}

#endif // JSONLDDOCUMENT_H
