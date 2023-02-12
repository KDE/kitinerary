/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "protobufstreamreader.h"

#include <QDebug>

using namespace KItinerary;

ProtobufStreamReader::ProtobufStreamReader(std::string_view data)
    : m_data(data)
{
}

ProtobufStreamReader::~ProtobufStreamReader() = default;

uint64_t ProtobufStreamReader::readVarint()
{
    uint64_t result = 0;
    int shift = 0;
    while (m_cursor < m_data.size()) {
        const uint64_t b = m_data[m_cursor++];
        result |= (b & 0b0111'1111) << shift;
        if ((b & 0b1000'0000) == 0) {
            break;
        }
        shift += 7;
    }

    return result;
}

uint64_t ProtobufStreamReader::peekVarint()
{
    auto prevCursor = m_cursor;
    const auto result = readVarint();
    m_cursor = prevCursor;
    return result;
}

uint64_t ProtobufStreamReader::fieldNumber()
{
    return peekVarint() >> 3;
}

ProtobufStreamReader::WireType ProtobufStreamReader::wireType()
{
    return static_cast<WireType>(peekVarint() & 0b111);
}

uint64_t ProtobufStreamReader::readVarintField()
{
    readVarint(); // skip field number and wire type
    return readVarint();
}

std::string_view ProtobufStreamReader::readLengthDelimitedRecord()
{
    if (wireType() != LEN) {
        return {};
    }
    readVarint(); // skip field number and wire type
    const auto len = readVarint();
    if (m_cursor + len <= m_data.size()) {
        auto data = std::string_view(m_data.begin() + m_cursor, len);
        m_cursor += len;
        return data;
    }
    return {};
}

QString ProtobufStreamReader::readString()
{
    const auto data = readLengthDelimitedRecord();
    return QString::fromUtf8(data.begin(), data.size());
}

ProtobufStreamReader ProtobufStreamReader::readSubMessage()
{
    return ProtobufStreamReader(readLengthDelimitedRecord());
}

bool ProtobufStreamReader::atEnd() const
{
    return m_cursor >= m_data.size();
}

void ProtobufStreamReader::skip()
{
    switch (wireType()) {
        case VARINT:
            readVarintField();
            break;
        case LEN:
            readLengthDelimitedRecord();
            break;
        case I64:
        case I32:
        case SGROUP:
        case EGROUP:
            qWarning() << "encountered deprecated or unsupported protobuf wire type!" << wireType();
            m_cursor = m_data.size();
            break;
    }
}
