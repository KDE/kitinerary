/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "uic9183header.h"
#include "logging.h"

#include <cstdint>

using namespace KItinerary;

enum {
    PrefixSize = 14,
    SignatureSizeV1 = 50,
    SignatureSizeV2 = 64,
    SuffixSize = 4,
    ZlibHeaderSize = 2
};

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
    const auto offset = PrefixSize + (version == 1 ? SignatureSizeV1 : SignatureSizeV2) + SuffixSize;
    if (data.size() < offset + ZlibHeaderSize) {
        return;
    }

    // check for zlib headers 0x789C or 0x78DA
    if ((uint8_t)data[offset] != 0x78 || ((uint8_t)data[offset + 1] != 0x9C && (uint8_t)data[offset + 1] != 0xDA)) {
        qCWarning(Log) << "UIC 918-3 payload has wrong zlib header.";
        return;
    }

    m_data = data;
}

bool Uic9183Header::isValid() const
{
    return !m_data.isEmpty();
}

int Uic9183Header::signatureSize() const
{
    switch (version()) {
        case 1: return 50;
        case 2: return 64;
    };
    return 0;
}

int Uic9183Header::compressedMessageSize() const
{
    return Uic9183Utils::readAsciiEncodedNumber(m_data, 14 + signatureSize(), 4);
}

int Uic9183Header::compressedMessageOffset() const
{
    return PrefixSize + (version() == 1 ? SignatureSizeV1 : SignatureSizeV2) + SuffixSize;
}
