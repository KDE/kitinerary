/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_BITVECTOR_H
#define KITINERARY_BITVECTOR_H

#include <bitset>
#include <cassert>
#include <string_view>
#include <type_traits>

class QByteArray;

namespace KItinerary {

/** Non-owning bit-level view for working with data that isn't byte-aligned. */
class BitVectorView
{
public:
    BitVectorView();
    explicit BitVectorView(std::string_view data);
    ~BitVectorView();

    using size_type = std::string_view::size_type;

    /** Size of this vector in bits. */
    size_type size() const;

    /** Returns the bit value at @p index. */
    uint8_t at(size_type index) const;

    /** Read a big endian unsigned number at bit offset @p index
     *  and @p bits in length.
     */
    template <typename T>
    T valueAtMSB(size_type index, size_type bits) const
    {
        static_assert(std::is_integral_v<T>);
        assert(size_type(sizeof(T) * 8) >= bits);

        T result = {};
        for (size_type i = 0; i < bits; ++i) {
            result <<= 1;
            result |= at(index + i);
        }

        return result;
    }

    /** Returns @p bytes starting at bit offset @p index. */
    QByteArray byteArrayAt(size_type index, size_type bytes) const;

    /** Reads a std::bitset from @p index. */
    template <std::size_t N>
    std::bitset<N> bitsetAt(size_type index) const
    {
        std::bitset<N> result = {};
        for (size_type i = 0; i < (size_type)N; ++i) {
            result[N - i - 1] = at(index + i);
        }
        return result;
    }

private:
    std::string_view m_data;
};

}

#endif // KITINERARY_BITVECTOR_H
