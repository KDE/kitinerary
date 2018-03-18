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

/** Similar to QExplicitlySharedDataPointer, but with support
 *  for use with our polymorphic types.
 */
template <typename T>
class shared_data_ptr
{
public:
    shared_data_ptr() = default;
    explicit shared_data_ptr(T *data)
        : d(data)
    {
        if (d) {
            d->ref.ref();
        }
    }

    inline shared_data_ptr(const shared_data_ptr<T> &other) : d(other.d)
    {
        if (d) {
            d->ref.ref();
        }
    }

    inline ~shared_data_ptr()
    {
        if (d && !d->ref.deref()) {
            delete d;
            d = nullptr;
        }
    }

    inline void detach()
    {
        if (d && d->ref.load() != 1) {
            auto copy = d->clone(d);
            copy->ref.ref();
            if (!d->ref.deref()) {
                delete d;
            }
            d = copy;
        }
    }

    inline shared_data_ptr& operator=(const shared_data_ptr &other)
    {
        if (other.d == d) {
            return *this;
        }
        if (other.d) {
            other.d->ref.ref();
        }
        T *old = d;
        d = other.d;
        if (old && !old->ref.deref()) {
            delete old;
        }
        return *this;
    }

    inline T *operator->() { return d; }
    inline const T *operator->() const { return d; }
    inline T* data() { return d; }
    inline const T* data() const { return d; }

private:
    T* d = nullptr;
};

}
}

#define KITINERARY_BASE_GADGET(Class) \
    Q_GADGET \
protected: \
    Class() = delete; \
    Class(const Class &other); \
    Class(Class ## Private *dd); \
    ~Class(); \
    Class& operator=(const Class &other) = delete; \
private:

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

#define KITINERARY_PROPERTY(Type, Name, SetName) \
    Q_PROPERTY(Type Name READ Name WRITE SetName STORED true) \
public: \
    Type Name() const; \
    void SetName(KItinerary::detail::parameter_type<Type>::type v); \
private:

#endif
