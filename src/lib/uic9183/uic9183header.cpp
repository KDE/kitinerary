/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "uic9183header.h"
#include "logging.h"

#include <cstdint>
#include <cstring>

using namespace KItinerary;

enum {
    PrefixSize = 14,
    SignatureSizeV1 = 50,
    SignatureSizeV2 = 64,
    SuffixSize = 4,
    ZlibHeaderSize = 2
};

static bool isZlibHeader(const QByteArray &data, int offset)
{
    // check for zlib headers 0x789C or 0x78DA
    return ((uint8_t)data[offset] == 0x78 && ((uint8_t)data[offset + 1] == 0x9C || (uint8_t)data[offset + 1] == 0xDA));
}

Uic9183Header::Uic9183Header() = default;
Uic9183Header::Uic9183Header(const QByteArray& data)
{
    if (data.size() < (PrefixSize + SignatureSizeV1 + SuffixSize + ZlibHeaderSize)) {
        return;
    }
    if (!data.startsWith("#UT") && !data.startsWith("OTI")) {
        return;
    }

    const auto version = Uic9183Utils::readAsciiEncodedNumber(data, 3, 2);
    if (version != 1 && version != 2) {
        return;
    }
    auto offset = PrefixSize + (version == 1 ? SignatureSizeV1 : SignatureSizeV2) + SuffixSize;
    if (data.size() < offset + ZlibHeaderSize) {
        return;
    }

    // compressedMessageOffset() contains workarounds for wrong version claims, which the above check doesn't do yet
    m_data = data;
    offset = compressedMessageOffset();
    if (!isZlibHeader(data, offset)) {
        qCWarning(Log) << "UIC 918-3 payload has wrong zlib header.";
        m_data.clear();
        return;
    }

    m_data = data;
}

bool Uic9183Header::operator==(const Uic9183Header &other) const
{
    return isValid() && other.isValid() && std::memcmp(m_data.constData(), other.m_data.constData(), PrefixSize) == 0;
}

bool Uic9183Header::isValid() const
{
    return !m_data.isEmpty();
}

int Uic9183Header::signatureSize() const
{
    switch (version()) {
        case 1: return SignatureSizeV1;
        case 2:
        {
            // workaround some tickets claiming version 2 but actually being version 1...
            if (isZlibHeader(m_data, PrefixSize + SignatureSizeV2 + SuffixSize)) {
                return SignatureSizeV2;
            }
            if (isZlibHeader(m_data, PrefixSize + SignatureSizeV1 + SuffixSize)) {
                return SignatureSizeV1;
            }
            return SignatureSizeV2;
        }
    };
    return 0;
}

int Uic9183Header::compressedMessageSize() const
{
    return Uic9183Utils::readAsciiEncodedNumber(m_data, PrefixSize + signatureSize(), 4);
}

int Uic9183Header::compressedMessageOffset() const
{
    return PrefixSize + signatureSize() + SuffixSize;
}
