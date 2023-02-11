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
