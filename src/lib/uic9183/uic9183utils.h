/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/


#pragma once

#include "kitinerary_export.h"

#include <QString>

namespace KItinerary {

class Uic9183Block;

/*
 * Low-level decoding utilities for UIC 918.3 ticket content.
 * @internal
 */
namespace Uic9183Utils
{
    /** Reads an ASCII encoded numerical value. */
    int readAsciiEncodedNumber(const char *data, int size, int offset, int length);
    KITINERARY_EXPORT int readAsciiEncodedNumber(const QByteArray &data, int offset, int length);
    KITINERARY_EXPORT int readAsciiEncodedNumber(const Uic9183Block &block, int offset, int length);

    /** Reads a UTF8 encoded string. */
    QString readUtf8String(const char *data, int size, int offset, int length);
    KITINERARY_EXPORT QString readUtf8String(const QByteArray &data, int offset, int length);
    KITINERARY_EXPORT QString readUtf8String(const Uic9183Block &block, int offset, int length);
}

#define UIC_NUM_PROPERTY(Name, Offset, Length) \
public: \
    inline int Name() const { return Uic9183Utils::readAsciiEncodedNumber(m_data, Offset, Length); } \
    Q_PROPERTY(int Name READ Name)

#define UIC_STR_PROPERTY(Name, Offset, Length) \
public: \
    inline QString Name() const { return Uic9183Utils::readUtf8String(m_data, Offset, Length); } \
    Q_PROPERTY(QString Name READ Name)
}

