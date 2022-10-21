/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_UPERDECODER_H
#define KITINERARY_UPERDECODER_H

#include "bitvectorview.h"

#include <QMetaEnum>

namespace KItinerary {

/** Decoder for data encoded according to X.691 ASN.1 Unaligned Packed Encoding Rules (UPER). */
class UPERDecoder
{
public:
    explicit UPERDecoder(BitVectorView data);
    ~UPERDecoder();

    using size_type = BitVectorView::size_type;

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

    /** Read an unconstrained IA5String (7 bit ASCII).
     *  @see X.691 §30
     */
    QByteArray readIA5String();

    /** Read length-constrained IA5String (7 bit ASCII).
     *  @see X.691 §30
     */
    QByteArray readIA5String(size_type minLenght, size_type maxLength);

    /** Read unconstrained octet string (8 bit data).
     * @see X.691 §17
     */
    QByteArray readOctetString();

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
    QList<int> readSequenceOfConstrainedWholeNumber(int64_t minimum, int64_t maximum);
    QList<int> readSequenceOfUnconstrainedWholeNumber();
    QList<QByteArray> readSequenceOfIA5String();

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

    /** Read a choice value.
     *  @see X.691 §23
     */
    template <typename... Ts>
    inline QVariant readChoiceWithExtensionMarker()
    {
        assert(!readBoolean()); // TODO
        constexpr auto count = sizeof...(Ts);
        const auto choiceIdx = readConstrainedWholeNumber(0, count - 1);
        assert(choiceIdx < (int)count); // TODO
        return readChoiceElement<Ts...>(choiceIdx);
    }

private:
    QByteArray readIA5StringData(size_type len);

    template <typename T, typename T1, typename... Ts>
    inline QVariant readChoiceElement(int choiceIdx)
    {
        if (choiceIdx == 0) {
            return readChoiceElement<T>(choiceIdx);
        }
        return readChoiceElement<T1, Ts...>(choiceIdx - 1);
    }
    template <typename T>
    inline QVariant readChoiceElement(int choiceIdx)
    {
        assert(choiceIdx == 0);
        T value;
        value.decode(*this);
        return QVariant::fromValue(value);
    }

    BitVectorView m_data;
    size_type m_idx = {};
};

}

#endif // KITINERARY_UPERDECODER_H
