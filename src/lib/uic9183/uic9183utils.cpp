/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "uic9183utils.h"
#include "uic9183block.h"
#include "logging.h"

using namespace KItinerary;

int Uic9183Utils::readAsciiEncodedNumber(const char* data, int size, int offset, int length)
{
    if (!data || offset < 0 || length < 1 || size < 1 || offset + length > size) {
        qCWarning(Log) << "Invalid UIC 918.3 read" << offset << length << size;
        return {};
    }

    int v = 0;
    for (int i = 0; i < length; ++i) {
        v *= 10;
        v += (*(data + offset + i)) - '0';
    }
    return v;
}

int Uic9183Utils::readAsciiEncodedNumber(const QByteArray &data, int offset, int length)
{
    return readAsciiEncodedNumber(data.constData(), data.size(), offset, length);
}

int Uic9183Utils::readAsciiEncodedNumber(const Uic9183Block &block, int offset, int length)
{
    return readAsciiEncodedNumber(block.content(), block.contentSize(), offset, length);
}

QString Uic9183Utils::readUtf8String(const char* data, int size, int offset, int length)
{
    if (length == 0) { // common in ÖBB RCT2 blocks...
        return {};
    }

    if (!data || offset < 0 || length < 1 || size < 1 || offset + length > size) {
        qCWarning(Log) << "Invalid UIC 918.3 read" << offset << length << size;
        return {};
    }

    return QString::fromUtf8(data + offset, length);
}

QString Uic9183Utils::readUtf8String(const QByteArray& data, int offset, int length)
{
    return readUtf8String(data.constData(), data.size(), offset, length);
}

QString Uic9183Utils::readUtf8String(const Uic9183Block &block, int offset, int length)
{
    return readUtf8String(block.content(), block.contentSize(), offset, length);
}
