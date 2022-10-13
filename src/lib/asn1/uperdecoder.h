/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_UPERDECODER_H
#define KITINERARY_UPERDECODER_H

#include "bitvector.h"

#include <QMetaEnum>

namespace KItinerary {

/** Decoder for data encoded according to X.691 ASN.1 Unaligned Packed Encoding Rules (UPER). */
class UPERDecoder
{
public:
    explicit UPERDecoder(const BitVector &data);
    ~UPERDecoder();

    using size_type = BitVector::size_type;

    size_type offset() const;
    void seek(size_type index);

    /** Read constrained whole number from the current position.
     *  @see X.691 §11.5
     */
    int64_t readConstrainedWholeNumber(int64_t minimum, int64_t maximum);

    /** Read unconstrained whole number.
     *  @see X.691 §11.8
     */
    int64_t readUnconstrainedWholeNumber();

    /** Read length determinant.
     * @see X.691 §11.9
     */
    size_type readLengthDeterminant();

    /** Read UTF-8 string. */
    QString readUtf8String();

    /** Read boolean value. */
    bool readBoolean();

    /** Read a IA5String (7 bit ASCII).
     *  @see X.691 §30
     */
    QByteArray readIA5String(size_type minLenght = 0, size_type maxLength = -1);

    /** Read an @tparam N sized bitmap. */
    template <std::size_t N>
    inline std::bitset<N> readBitset()
    {
        auto result = m_data.bitsetAt<N>(m_idx);
        m_idx += N;
        return result;
    }

    /** Read a sequence-of field with unrestricted size.
     *  @see X.691 §20
     */
    template <typename T>
    inline QList<T> readSequenceOf()
    {
        const auto size = readLengthDeterminant();
        QList<T> result;
        result.reserve(size);
        for (size_type i = 0; i < size; ++i) {
            T element;
            element.decode(*this);
            result.push_back(std::move(element));
        }
        return result;
    }

    /** Read enumerated value.
     *  @see X.691 §14
     */
    template <typename T>
    inline T readEnumerated()
    {
        const auto me = QMetaEnum::fromType<T>();
        const auto idx = readConstrainedWholeNumber(0, me.keyCount() - 1);
        return static_cast<T>(me.value(idx));
    }
    template <typename T>
    inline T readEnumeratedWithExtensionMarker()
    {
        assert(!readBoolean()); // TODO
        return readEnumerated<T>();
    }

private:
    BitVector m_data;
    size_type m_idx = {};
};

}

#endif // KITINERARY_UPERDECODER_H
