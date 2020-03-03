/*
    Copyright (C) 2020 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef KITINERARY_BER_ELEMENT_P_H
#define KITINERARY_BER_ELEMENT_P_H

#include <QByteArray>

#include <cstdint>

namespace KItinerary {

/** BER/DER/X.690 encoding classes and functions. */
namespace BER {

/**
 * An element in BER/DER/X.690 encoding.
 * Implicitly this is also kinda implementing a QByteArrayRef, as this works without copying
 * the underlying data.
 */
class Element
{
public:
    Element();
    explicit Element(const QByteArray &data, int offset = 0, int size = -1);
    ~Element();

    /** Returns @c true if this element has a valid structure and can be read from. */
    bool isValid() const;

    /** Type, "right-aligned" in the returned 32bit value. */
    uint32_t type() const;

    /** Size of the entire element (type, size and content). */
    int size() const;

    /** Size of the value part of this element.
     *  This is excluding a possible variable length end marker.
     */
    int contentSize() const;
    /** Raw content data. */
    const uint8_t* contentData() const;

    /** Convenience method to access typed content. */
    template <typename T>
    inline const T* contentAt(int offset) const
    {
        return reinterpret_cast<const T*>(contentData() + offset);
    }

    /** First child element, for nested types. */
    Element first() const;
    /** Next child element, for nested types. */
    Element next(const Element &prev) const;

private:
    int typeSize() const;
    int lengthSize() const;
    int contentOffset() const;

    QByteArray m_data;
    int m_offset = -1;
    int m_dataSize = -1;
};

template <uint32_t TagValue>
struct TypedElement : public Element
{
    inline bool isValid() const
    {
        return Element::isValid() && type() == TagValue;
    }
};

}
}

#endif // KITINERARY_BER_ELEMENT_P_H
