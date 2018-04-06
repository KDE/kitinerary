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

#ifndef KITINERARY_DATATYPES_H
#define KITINERARY_DATATYPES_H

#include <QMetaType>
#include <QSharedDataPointer>

#include <type_traits>

class QString;

namespace KItinerary {
namespace detail {

template <typename T>
struct parameter_type
{
    using type = typename std::conditional<std::is_fundamental<T>::value, T, const T&>::type;
};

}
}

#define KITINERARY_GADGET(Class) \
    Q_GADGET \
    Q_PROPERTY(QString className READ className STORED false CONSTANT) \
    QString className() const; \
public: \
    Class(); \
    Class(const Class &other); \
    ~Class(); \
    Class& operator=(const Class &other); \
private:

#define KITINERARY_BASE_GADGET(Class) \
    KITINERARY_GADGET(Class) \
protected: \
    Class(Class ## Private *dd); \
private:

#define KITINERARY_PROPERTY(Type, Name, SetName) \
    Q_PROPERTY(Type Name READ Name WRITE SetName STORED true) \
public: \
    Type Name() const; \
    void SetName(KItinerary::detail::parameter_type<Type>::type v); \
private:

#endif
