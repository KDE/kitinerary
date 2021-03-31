/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

class QIODevice;

namespace KItinerary {
namespace Generator {

/** Generate IANA timezone lookup table. */
class TimezoneDbGenerator
{
public:
    void generate(QIODevice *out);
    void generateHeader(QIODevice *out);
};

}
}

