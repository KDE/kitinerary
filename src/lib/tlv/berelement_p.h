/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_BER_ELEMENT_P_H
#define KITINERARY_BER_ELEMENT_P_H

#include "kitinerary_export.h"

#include <QByteArray>

#include <cstdint>

class QIODevice;

namespace KItinerary {

/** BER/DER/X.690 encoding classes and functions. */
namespace BER {

/**
 * An element in BER/DER/X.690 encoding.
 * Implicitly this is also kinda implementing a QByteArrayRef, as this works without copying
 * the underlying data.
 */
class KITINERARY_EXPORT Element
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
    /** Raw data of this element.
     *  Typically only needed when copying/writing this element somewhere.
     */
    const char* rawData() const;

    /** Size of the value part of this element.
     *  This is excluding a possible variable length end marker.
     */
    int contentSize() const;
    /** Raw content data. */
    const uint8_t* contentData() const;

    /** Convenience method to access typed content. */
    template <typename T>
    inline const T* contentAt(int offset = 0) const
    {
        if (offset < 0 || (int)sizeof(T) > contentSize() - offset) {
            return nullptr;
        }
        return reinterpret_cast<const T*>(contentData() + offset);
    }

    /** First child element, for nested types. */
    Element first() const;
    /** Next child element, for nested types. */
    Element next() const;
    /** Returns the first child element of the given @p type. */
    Element find(uint32_t type) const;

    /** Writes the given size in BER encoding to @p out. */
    static void writeSize(QIODevice *out, int size);

private:
    int typeSize() const;
    int lengthSize() const;
    int contentOffset() const;

    QByteArray m_data;
    int m_offset = -1;
    int m_dataSize = -1;
};

template <uint32_t TagValue>
class TypedElement : public Element
{
public:
    using Element::Element;
    inline bool isValid() const
    {
        return Element::isValid() && type() == TagValue;
    }
};

}
}

#endif // KITINERARY_BER_ELEMENT_P_H
