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
#include <QLocale>
#include <QTimeZone>

namespace KItinerary {

#define KITINERARY_PRIVATE_ABSTRACT_GADGET(Class) \
public: \
virtual ~ Class ## Private() = default; \
virtual Class ## Private * clone() const = 0; \
typedef Class ## Private base_type; \
private: \

#define KITINERARY_PRIVATE_GADGET(Class) \
public: \
inline base_type* clone() const override { \
    return new Class ## Private(*this); \
} \
private:


#define KITINERARY_MAKE_SIMPLE_CLASS(Class) \
Class::Class() : d(new Class ## Private) {} \
Class::Class(const Class &other) = default; \
Class::~Class() = default; \
Class& Class::operator=(const Class &other) { d = other.d; return *this; } \
QString Class::className() const { return QStringLiteral(#Class); }

#define KITINERARY_MAKE_ABSTRACT_CLASS(Class) \
Class::Class(const Class &other) = default; \
Class::Class(Class ## Private *dd) : d(dd) {} \
Class::~Class() = default;

#define KITINERARY_MAKE_SUB_CLASS(Class, Base) \
Class::Class() : Base(new Class ## Private) {} \
Class::Class(const Class &other) = default; \
Class::~Class() = default; \
Class& Class::operator=(const Class &other) { d = other.d; return *this; } \
QString Class::className() const { return QStringLiteral(#Class); }

#define K_D(Class) auto d = static_cast<Class ## Private *>(this->d.data())

#define KITINERARY_MAKE_PROPERTY(Class, Type, Name, SetName) \
Type Class::Name() const { return static_cast<const Class ## Private*>(d.data())->Name; } \
void Class::SetName(detail::parameter_type<Type>::type value) { \
    d.detach(); \
    static_cast<Class ## Private*>(d.data())->Name = value; \
}

static inline QString localizedDateTime(const QDateTime &dt)
{
    auto s = QLocale().toString(dt, QLocale::ShortFormat);
    if (dt.timeSpec() == Qt::TimeZone || dt.timeSpec() == Qt::OffsetFromUTC) {
        s += QLatin1Char(' ') + dt.timeZone().abbreviation(dt);
    }
    return s;
}

}

#endif
