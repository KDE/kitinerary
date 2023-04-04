/*
   SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kitinerary_export.h"

#include <QList>
#include <QVariant>

class QJsonArray;
class QJsonObject;
struct QMetaObject;

namespace KItinerary {

namespace JsonLd {

/** Checks whether @p v holds a null-like value.
 *  This is similar to QVariant::isNull in Qt5, but differs
 *  from the "shallow" QVariant::isNull in Qt6 which doesn't
 *  check the content at all.
 */
KITINERARY_EXPORT bool valueIsNull(const QVariant &v);

}

/** Serialization/deserialization code for JSON-LD data.
 *  @see https://www.w3.org/TR/json-ld/
 */
class JsonLdDocument {

public:
    /** Convert JSON-LD array into instantiated data types. */
  static KITINERARY_EXPORT QList<QVariant> fromJson(const QJsonArray &array);
  /** Convert JSON-LD object into an instantiated data type. */
  static KITINERARY_EXPORT QList<QVariant> fromJson(const QJsonObject &obj);

  /** Convert a single JSON-LD object into an instantiated data type.
   *  @note Use this only if you are sure the JSON-LD object does not expand to
   * multiple objects! That is usually only the case for objects you have
   * written yourself and that semantically are guaranteed to be a single
   * object. Anything received from external sources can expand and should not
   * use this method.
   *  @since 20.04
   */
  static KITINERARY_EXPORT QVariant fromJsonSingular(const QJsonObject &obj);

  /** Serialize instantiated data types to JSON-LD. */
  static KITINERARY_EXPORT QJsonArray toJson(const QList<QVariant> &data);
  /** Serialize instantiated data type to JSON-LD. */
  static KITINERARY_EXPORT QJsonObject toJson(const QVariant &data);

  /** JSON-LD serrialization of an invidividual data value.
   *  Unlike the above this also works with primitive types.
   */
  static KITINERARY_EXPORT QJsonValue toJsonValue(const QVariant &data);

  /** Read property @p name on object @p obj. */
  static KITINERARY_EXPORT QVariant readProperty(const QVariant &obj,
                                                 const char *name);
  /** Set property @p name on object @p obj to value @p value. */
  static KITINERARY_EXPORT void writeProperty(QVariant &obj, const char *name,
                                              const QVariant &value);
  /** Set property @p name on object @p obj to value @p value. */
  template <typename T>
  inline static void writeProperty(T &obj, const char *name,
                                   const QVariant &value);

  /** Removes property @p name on object @p obj. */
  KITINERARY_EXPORT static void removeProperty(QVariant &obj, const char *name);

  /** Apply all properties of @p rhs on to @p lhs.
   *  Use this to merge two top-level objects of the same type, with
   *  @p rhs containing newer information.
   */
  KITINERARY_EXPORT static QVariant apply(const QVariant &lhs,
                                          const QVariant &rhs);

  /** Register a custom type for deserialization. */
  template <typename T> static inline void registerType() {
    registerType(T::typeName(), &T::staticMetaObject, qMetaTypeId<T>());
    }

private:
    KITINERARY_EXPORT static void writePropertyImpl(const QMetaObject *mo, void *obj, const char *name, const QVariant &value);
    KITINERARY_EXPORT static void registerType(const char *typeName, const QMetaObject *mo, int metaTypeId);
};

template <typename T>
inline void JsonLdDocument::writeProperty(T &obj, const char *name, const QVariant &value)
{
    writePropertyImpl(&T::staticMetaObject, &obj, name, value);
}

}

