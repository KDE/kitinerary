/*
   SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_JSONLDDOCUMENT_H
#define KITINERARY_JSONLDDOCUMENT_H

#include "kitinerary_export.h"

#include <QVariant>
#include <QVector>

class QJsonArray;
class QJsonObject;
struct QMetaObject;

namespace KItinerary {

/** Serialization/deserialization code for JSON-LD data.
 *  @see https://www.w3.org/TR/json-ld/
 */
class JsonLdDocument {

public:
    /** Convert JSON-LD array into instantiated data types. */
    static KITINERARY_EXPORT QVector<QVariant> fromJson(const QJsonArray &array);
    /** Convert JSON-LD object into an instantiated data type. */
    static KITINERARY_EXPORT QVariant fromJson(const QJsonObject &obj); // TODO return QVector<QVariant> here

    /** Convert a single JSON-LD object into an instantiated data type.
     *  @note Use this only if you are sure the JSON-LD object does not expand to multiple objects!
     *  That is usually only the case for objects you have written yourself and that semantically
     *  are guaranteed to be a single object. Anything received from external sources can expand
     *  and should not use this method.
     *  @since 20.04
     */
    static KITINERARY_EXPORT QVariant fromJsonSingular(const QJsonObject &obj);

    /** Serialize instantiated data types to JSON-LD. */
    static KITINERARY_EXPORT QJsonArray toJson(const QVector<QVariant> &data);
    /** Serialize instantiated data type to JSON-LD. */
    static KITINERARY_EXPORT QJsonObject toJson(const QVariant &data);

    /** Read property @p name on object @p obj. */
    static KITINERARY_EXPORT QVariant readProperty(const QVariant &obj, const char *name);
    /** Set property @p name on object @p obj to value @p value. */
    static KITINERARY_EXPORT void writeProperty(QVariant &obj, const char *name, const QVariant &value);
    /** Set property @p name on object @p obj to value @p value. */
    template <typename T>
    inline static void writeProperty(T &obj, const char *name, const QVariant &value);

    /** Removes property @p name on object @p obj. */
    KITINERARY_EXPORT static void removeProperty(QVariant &obj, const char *name);

    /** Apply all properties of @p rhs on to @p lhs.
    *  Use this to merge two top-level objects of the same type, with
    *  @p rhs containing newer information.
    */
    KITINERARY_EXPORT static QVariant apply(const QVariant &lhs, const QVariant &rhs);

private:
    KITINERARY_EXPORT static void writePropertyImpl(const QMetaObject *mo, void *obj, const char *name, const QVariant &value);
};

template <typename T>
inline void JsonLdDocument::writeProperty(T &obj, const char *name, const QVariant &value)
{
    writePropertyImpl(&T::staticMetaObject, &obj, name, value);
}

}

#endif // KITINERARY_JSONLDDOCUMENT_H