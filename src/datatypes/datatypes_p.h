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

#ifndef KITINERARY_DATATYPES_P_H
#define KITINERARY_DATATYPES_P_H

#include <QDateTime>

namespace KItinerary {

namespace detail {

// Helper types for the auto-generated operator==
// This is based on the approach described here https://woboq.com/blog/verdigris-implementation-tricks.html

// numerical index of properties, done in a way that we can daisy-chain overloads with it
template <int N = 255> struct num : public num<N - 1> {
    static constexpr int value = N;
    static constexpr num<N - 1> prev() { return {}; }
};
template <> struct num<0> { static constexpr int value = 0; };

// type tag, to avoid unwanted overload resolution on arguments other than num<>
template <typename T> struct tag {};

// SFINAE helper to determine if we have a polimorphic or a simple value type
template <typename T>
struct base_type {
    template <typename U> static typename U::super_type test(typename U::super_type*);
    template <typename U> static T test(...);
    typedef decltype(test<T>(nullptr)) type;
    static constexpr const bool is_valid = !std::is_same<type, T>::value;
};

// customization hook for comparisson for certain types
template <typename T> inline bool equals(typename parameter_type<T>::type lhs, typename parameter_type<T>::type rhs)
{
    return lhs == rhs;
}

// QDateTime::operator== is true for two instances referring to the same point in time
// we however want to know if two instances contain exactly the same information
template <> inline bool equals<QDateTime>(const QDateTime &lhs, const QDateTime &rhs)
{
    return lhs.timeSpec() == rhs.timeSpec() && lhs == rhs;
}

// QString::operator== ignores null vs empty
// we probably don't care either, but until that's decided this makes the existing tests pass
template <> inline bool equals<QString>(const QString &lhs, const QString &rhs)
{
    if (lhs.isEmpty() && rhs.isEmpty()) {
        return lhs.isNull() == rhs.isNull();
    }
    return lhs == rhs;
}

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
Class::Class(const Class&) = default; \
Class::~Class() = default; \
Class& Class::operator=(const Class &other) { d = other.d; return *this; } \
QString Class::className() const { return QStringLiteral(#Class); } \
Class::operator QVariant() const { return QVariant::fromValue(*this); } \
static_assert(sizeof(Class) == sizeof(void*), "dptr must be the only member!"); \
namespace detail { \
    static constexpr int property_counter(num<0>, tag<Class>) { return 1; } \
    static constexpr bool property_equals(num<0>, tag<Class ## Private>, const Class ## Private *, const Class ## Private *) { return true; } \
}

#define KITINERARY_MAKE_SIMPLE_CLASS(Class) \
KITINERARY_MAKE_CLASS_IMPL(Class) \
Class::Class() : d(*s_ ## Class ## _shared_null()) {}

#define KITINERARY_MAKE_BASE_CLASS(Class) \
KITINERARY_MAKE_CLASS_IMPL(Class) \
Class::Class() : d(*s_ ## Class ## _shared_null()) {} \
Class::Class(Class ## Private *dd) : d(dd) {}

#define KITINERARY_MAKE_SUB_CLASS(Class, Base) \
KITINERARY_MAKE_CLASS_IMPL(Class) \
Class::Class() : Base(s_ ## Class ## _shared_null()->data()) {}

#define K_D(Class) auto d = static_cast<Class ## Private *>(this->d.data())

#define KITINERARY_MAKE_PROPERTY(Class, Type, Name, SetName) \
Type Class::Name() const { return static_cast<const Class ## Private*>(d.data())->Name; } \
void Class::SetName(detail::parameter_type<Type>::type value) { \
    d.detach(); \
    static_cast<Class ## Private*>(d.data())->Name = value; \
} \
namespace detail { \
    static constexpr int property_counter(num<property_counter(num<>(), tag<Class>())> n, tag<Class>) { return decltype(n)::value + 1; } \
    static inline bool property_equals(num<property_counter(num<>(), tag<Class>())> n, tag<Class ## Private>, const Class ## Private *lhs, const Class ## Private *rhs) \
    { \
        if (equals<Type>(lhs->Name, rhs->Name)) { return property_equals(n.prev(), tag<Class ## Private>(), lhs, rhs); } \
        return false; \
    } \
}

#define KITINERARY_MAKE_OPERATOR(Class) \
bool Class::operator==(const Class &other) const \
{ \
    typedef Class ## Private this_type; \
    const auto lhs = static_cast<const this_type *>(d.data()); \
    const auto rhs = static_cast<const this_type*>(other.d.data()); \
    if (lhs == rhs) { \
        return true; \
    } \
    if (!detail::property_equals(detail::num<>(), detail::tag<this_type>(), lhs, rhs)) { \
        return false; \
    } \
    if (detail::base_type<this_type>::is_valid) { \
        typedef typename detail::base_type<this_type>::type super_type; \
        return detail::property_equals(detail::num<>(), detail::tag<super_type>(), static_cast<const super_type*>(lhs), static_cast<const super_type*>(rhs)); \
    } \
    return true; \
}

}

#endif
