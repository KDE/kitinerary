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

namespace KItinerary {

#define KITINERARY_PRIVATE_BASE_GADGET(Class) \
public: \
virtual ~ Class ## Private() = default; \
virtual Class ## Private * clone() const { \
    return new Class ##Private(*this); \
} \
typedef Class ## Private base_type; \
private: \

#define KITINERARY_PRIVATE_GADGET(Class) \
public: \
inline base_type* clone() const override { \
    return new Class ## Private(*this); \
} \
private:

#define KITINERARY_MAKE_CLASS_IMPL(Class) \
Q_GLOBAL_STATIC_WITH_ARGS(QExplicitlySharedDataPointer<Class ## Private>, s_ ## Class ## _shared_null, (new Class ## Private)) \
Class::Class(const Class&) = default; \
Class::~Class() = default; \
Class& Class::operator=(const Class &other) { d = other.d; return *this; } \
QString Class::className() const { return QStringLiteral(#Class); } \
Class::operator QVariant() const { return QVariant::fromValue(*this); } \
static_assert(sizeof(Class) == sizeof(void*), "dptr must be the only member!"); \

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
}

}

#endif
