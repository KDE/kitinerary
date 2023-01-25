/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "internal/instance_counter.h"
#include "internal/parameter_type.h"
#include "internal/strict_equal.h"

namespace KItinerary {

namespace detail {

// SFINAE helper to determine if we have a polymorphic or a simple value type
template <typename T>
struct base_type {
    template <typename U> static typename U::super_type test(typename U::super_type*);
    template <typename U> static T test(...);
    using type = decltype(test<T>(nullptr));
    static constexpr const bool is_valid = !std::is_same<type, T>::value;
};
}

#define KITINERARY_PRIVATE_BASE_GADGET(Class) \
public: \
virtual ~ Class ## Private() = default; \
virtual Class ## Private * clone() const { \
    return new Class ##Private(*this); \
} \
typedef Class ## Private base_type; \
typedef Class ## Private this_type; \
private: \

#define KITINERARY_PRIVATE_GADGET(Class) \
public: \
inline base_type* clone() const override { \
    return new Class ## Private(*this); \
} \
typedef this_type super_type; \
typedef Class ## Private this_type; \
private:

#define KITINERARY_MAKE_CLASS_IMPL(Class) \
Q_GLOBAL_STATIC_WITH_ARGS(QExplicitlySharedDataPointer<Class ## Private>, s_ ## Class ## _shared_null, (new Class ## Private)) \
Class::Class() : Class(s_ ## Class ## _shared_null()->data()) {} \
Class::Class(const Class&) = default; \
Class::~Class() = default; \
Class& Class::operator=(const Class &other) { d = other.d; return *this; } \
QString Class::className() const { return QStringLiteral(#Class); } \
Class::operator QVariant() const { return QVariant::fromValue(*this); } \
const char* Class::typeName() { return #Class; } \
static_assert(sizeof(Class) == sizeof(void*), "dptr must be the only member!"); \
namespace detail { \
    static constexpr int property_counter(num<0>, tag<Class>) { return 1; } \
    static constexpr bool property_equals(num<0>, tag<Class ## Private>, const Class ## Private *, const Class ## Private *) { return true; } \
}

#define KITINERARY_MAKE_CLASS(Class) \
KITINERARY_MAKE_CLASS_IMPL(Class) \
Class::Class(Class ## Private *dd) : d(dd) {}

#define KITINERARY_MAKE_DERIVED_CLASS(Class, Base) \
KITINERARY_MAKE_CLASS_IMPL(Class) \
Class::Class(Class ## Private *dd) : Base(dd) {}

#define KITINERARY_MAKE_PROPERTY(Class, Type, Name, SetName) \
Type Class::Name() const { return static_cast<const Class ## Private*>(d.data())->Name; } \
void Class::SetName(detail::parameter_type<Type>::type value) { \
    if (detail::strict_equal<Type>(static_cast<Class ## Private*>(d.data())->Name, value)) { return; } \
    d.detach(); \
    static_cast<Class ## Private*>(d.data())->Name = value; \
} \
namespace detail { \
    static constexpr int property_counter(num<property_counter(num<>(), tag<Class>())> n, tag<Class>) { return decltype(n)::value + 1; } \
    static inline bool property_equals(num<property_counter(num<>(), tag<Class>())> n, tag<Class ## Private>, const Class ## Private *lhs, const Class ## Private *rhs) \
    { \
        if (strict_equal<Type>(lhs->Name, rhs->Name)) { return property_equals(n.prev(), tag<Class ## Private>(), lhs, rhs); } \
        return false; \
    } \
}

#define KITINERARY_MAKE_OPERATOR(Class) \
bool Class::operator==(const Class &other) const \
{ \
    static_assert(detail::property_counter(detail::num<0>(), detail::tag<Class>()) == 1, "silence unused function warnings"); \
    typedef Class ## Private this_type; \
    const auto lhs = static_cast<const this_type *>(d.data()); \
    const auto rhs = static_cast<const this_type*>(other.d.data()); \
    if (lhs == rhs) { \
        return true; \
    } \
    if (!detail::property_equals(detail::num<>(), detail::tag<this_type>(), lhs, rhs)) { \
        return false; \
    } \
    if constexpr (detail::base_type<this_type>::is_valid) { \
        typedef typename detail::base_type<this_type>::type super_type; \
        return detail::property_equals(detail::num<>(), detail::tag<super_type>(), static_cast<const super_type*>(lhs), static_cast<const super_type*>(rhs)); \
    } \
    return true; \
}

}

