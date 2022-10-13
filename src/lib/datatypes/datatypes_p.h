/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QDateTime>
#include <QTimeZone>

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

// SFINAE helper to determine if we have a polymorphic or a simple value type
template <typename T>
struct base_type {
    template <typename U> static typename U::super_type test(typename U::super_type*);
    template <typename U> static T test(...);
    using type = decltype(test<T>(nullptr));
    static constexpr const bool is_valid = !std::is_same<type, T>::value;
};

// customization hook for comparison for certain types
template <typename T> inline bool equals(typename parameter_type<T>::type lhs, typename parameter_type<T>::type rhs)
{
    return lhs == rhs;
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
// compare QVariant contents (no longer the default with Qt6)
template <> inline bool equals<QVariant>(const QVariant &lhs, const QVariant &rhs)
{
    return lhs.isNull() == rhs.isNull() && (lhs.isNull() || QVariant::compare(lhs, rhs) == QPartialOrdering::Equivalent);
}
#endif

// QDateTime::operator== is true for two instances referring to the same point in time
// we however want to know if two instances contain exactly the same information
template <> inline bool equals<QDateTime>(const QDateTime &lhs, const QDateTime &rhs)
{
    if (lhs.timeSpec() != rhs.timeSpec() || lhs != rhs) {
        return false;
    }
    return lhs.timeSpec() == Qt::TimeZone ? lhs.timeZone() == rhs.timeZone() : true;
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
const char* Class::typeName() { return #Class; } \
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

#define KITINERARY_MAKE_INTERMEDIATE_CLASS(Class, Base) \
KITINERARY_MAKE_CLASS_IMPL(Class) \
Class::Class() : Base(s_ ## Class ## _shared_null()->data()) {} \
Class::Class(Class ## Private *dd) : Base(dd) {}

#define KITINERARY_MAKE_SUB_CLASS(Class, Base) \
KITINERARY_MAKE_CLASS_IMPL(Class) \
Class::Class() : Base(s_ ## Class ## _shared_null()->data()) {}

#define K_D(Class) auto d = static_cast<Class ## Private *>(this->d.data())

#define KITINERARY_MAKE_PROPERTY(Class, Type, Name, SetName) \
Type Class::Name() const { return static_cast<const Class ## Private*>(d.data())->Name; } \
void Class::SetName(detail::parameter_type<Type>::type value) { \
    if (detail::equals<Type>(static_cast<Class ## Private*>(d.data())->Name, value)) { return; } \
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
    if (detail::base_type<this_type>::is_valid) { \
        typedef typename detail::base_type<this_type>::type super_type; \
        return detail::property_equals(detail::num<>(), detail::tag<super_type>(), static_cast<const super_type*>(lhs), static_cast<const super_type*>(rhs)); \
    } \
    return true; \
}

}

