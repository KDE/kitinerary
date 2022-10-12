/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "uperdecoder.h"

#include <bit>

using namespace KItinerary;

UPERDecoder::UPERDecoder(const BitVector &data)
    : m_data(data)
{
}

UPERDecoder::~UPERDecoder() = default;

UPERDecoder::size_type UPERDecoder::offset() const
{
    return m_idx;
}

void UPERDecoder::seek(UPERDecoder::size_type index)
{
    m_idx = index;
}

int64_t UPERDecoder::readConstrainedWholeNumber(int64_t minimum, int64_t maximum)
{
    assert(minimum <= maximum);
    const uint64_t range = maximum - minimum + 1;
    if (range == 1) {
        return 0;
    }
    const size_type bits = 64 - std::countl_zero(range);
    const auto result = m_data.valueAtMSB<int64_t>(m_idx, bits);
    m_idx += bits;
    return result + minimum;
}

UPERDecoder::size_type UPERDecoder::readLengthDeterminant()
{
    size_type len = m_data.valueAtMSB<size_type>(m_idx, 8);
    m_idx += 8;
    if ((len & 0x80) == 0x00) {
        return len;
    }

    // TODO
    assert(false);
    return 0;
}

QString UPERDecoder::readUtf8String()
{
    const auto len = readLengthDeterminant();
    const auto res = QString::fromUtf8(m_data.byteArrayAt(m_idx, len));
    m_idx += len * 8;
    return res;
}

bool UPERDecoder::readBoolean()
{
    return m_data.at(m_idx++) != 0;
}

QByteArray UPERDecoder::readIA5String(size_type minLength, size_type maxLength)
{
    size_type len = 0;
    if (minLength == maxLength) {
        len = minLength;
    } else if (maxLength > 0) {
        len = readConstrainedWholeNumber(minLength, maxLength);
    } else {
        len = readLengthDeterminant();
    }

    QByteArray result;
    result.reserve(len);
    for (size_type i = 0; i < len; ++i) {
        const auto c = m_data.valueAtMSB<char>(m_idx, 7);
        m_idx += 7;
        result.push_back(c);
    }

    return result;
}
