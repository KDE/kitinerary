/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "uperdecoder.h"

#include <bit>

using namespace KItinerary;

UPERDecoder::UPERDecoder(BitVectorView data)
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
    const uint64_t range = maximum - minimum;
    const size_type bits = 64 - std::countl_zero(range);
    const auto result = m_data.valueAtMSB<int64_t>(m_idx, bits);
    m_idx += bits;
    return result + minimum;
}

int64_t UPERDecoder::readUnconstrainedWholeNumber()
{
    const auto len = readLengthDeterminant();
    if (len > 8) {
        setError("Encountered INTEGER bigger than 64bit, not implemented.");
        return 0;
    }
    int64_t result = m_data.valueAtMSB<int64_t>(m_idx, 8 * len);
    m_idx += 8 * len;
    return result; // TODO convert negative numbers
}

UPERDecoder::size_type UPERDecoder::readLengthDeterminant()
{
    size_type len = m_data.valueAtMSB<size_type>(m_idx, 8);
    m_idx += 8;
    if ((len & 0x80) == 0x00) {
        return len;
    }

    setError("Encountered not implemented length determinant variant.");
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

QByteArray UPERDecoder::readIA5StringData(size_type len)
{
    QByteArray result;
    result.reserve(len);
    for (size_type i = 0; i < len; ++i) {
        const auto c = m_data.valueAtMSB<char>(m_idx, 7);
        m_idx += 7;
        result.push_back(c);
    }
    return result;
}

QByteArray UPERDecoder::readIA5String()
{
    return readIA5StringData(readLengthDeterminant());
}

QByteArray UPERDecoder::readIA5String(size_type minLength, size_type maxLength)
{
    size_type len = 0;
    if (minLength == maxLength) {
        len = minLength;
    } else {
        len = readConstrainedWholeNumber(minLength, maxLength);
    }
    return readIA5StringData(len);
}

QByteArray UPERDecoder::readOctetString()
{
    const auto len = readLengthDeterminant();
    const auto res = m_data.byteArrayAt(m_idx, len);
    m_idx += len * 8;
    return res;
}

QList<int> UPERDecoder::readSequenceOfConstrainedWholeNumber(int64_t minimum, int64_t maximum)
{
   const auto size = readLengthDeterminant();
    QList<int> result;
    result.reserve(size);
    for (size_type i = 0; i < size; ++i) {
        result.push_back(readConstrainedWholeNumber(minimum, maximum));
    }
    return result;
}

QList<int> UPERDecoder::readSequenceOfUnconstrainedWholeNumber()
{
   const auto size = readLengthDeterminant();
    QList<int> result;
    result.reserve(size);
    for (size_type i = 0; i < size; ++i) {
        result.push_back(readUnconstrainedWholeNumber());
    }
    return result;
}

QList<QByteArray> UPERDecoder::readSequenceOfIA5String()
{
    const auto size = readLengthDeterminant();
    QList<QByteArray> result;
    result.reserve(size);
    for (size_type i = 0; i < size; ++i) {
        result.push_back(readIA5String());
    }
    return result;
}

QList<QString> UPERDecoder::readSequenceOfUtf8String()
{
    const auto size = readLengthDeterminant();
    QList<QString> result;
    result.reserve(size);
    for (size_type i = 0; i < size; ++i) {
        result.push_back(readUtf8String());
    }
    return result;
}

bool UPERDecoder::hasError() const
{
    return !m_error.isEmpty();
}

QByteArray UPERDecoder::errorMessage() const
{
    return m_error;
}

void UPERDecoder::setError(const char *msg)
{
    m_error = "uPER decoding error at offset " + QByteArray::number((qint64)m_idx) + ": " + msg;
}
