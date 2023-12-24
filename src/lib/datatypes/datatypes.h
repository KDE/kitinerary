/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "internal/parameter_type.h"

#include <QMetaType>
#include <QSharedDataPointer>
#include <QVariant>

class QString;

namespace KItinerary {

/** JSON-LD data type helper functions. */
namespace JsonLd {

/** Returns @c true if @p value is of type @p T. */
template <typename T>
inline bool isA(const QVariant &value)
{
    return value.userType() == qMetaTypeId<T>();
}

/** Checks if the given value can be up-cast to @p T */
template <typename T>
inline bool canConvert(const QVariant &value)
{
    const auto mt = QMetaType(value.userType());
    // for enums/flags, this is the enclosing meta object starting with Qt6!
    const auto mo = mt.metaObject();
    if ((mt.flags() & QMetaType::IsGadget) == 0 || !mo) {
        return false;
    }
    return mo->inherits(&T::staticMetaObject);
}

/** Up-cast @p value to @p T.
 *  @note This does not perform any safety checks!
 *  @see canConvert
 */
template <typename T>
inline T convert(const QVariant &value)
{
    return T(*static_cast<const T*>(value.constData()));
}

}
}

/** Macro to mark a vocabulary type class.
 *  Adds Qt introspection, value type semantics, comparison operators and additional
 *  type information needed for JSON-LD (de)serialziation and QML access.
 */
#define KITINERARY_GADGET(Class) \
    Q_GADGET \
    Q_PROPERTY(QString className READ className STORED false CONSTANT) \
    QString className() const; \
public: \
    Class(); \
    Class(const Class &other); \
    ~Class(); \
    Class& operator=(const Class &other); \
    bool operator<(const Class &other) const; \
    bool operator==(const Class &other) const; \
    inline bool operator!=(const Class &other) const { return !(*this == other); } \
    operator QVariant () const; \
    static const char* typeName(); \
protected: \
    Class(Class ## Private *dd); \
private:

/** Macro to add a vocabulary type property.
 *  This adds Qt introspection and declarations for getter and setter methods.
 */
#define KITINERARY_PROPERTY(Type, Name, SetName) \
    Q_PROPERTY(Type Name READ Name WRITE SetName STORED true) \
public: \
    Type Name() const; \
    void SetName(KItinerary::detail::parameter_type<Type>::type value); \
private:

