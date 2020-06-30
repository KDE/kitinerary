/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_TIMEZONEDBGENERATOR_H
#define KITINERARY_TIMEZONEDBGENERATOR_H

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

#endif // KITINERARY_TIMEZONEDBGENERATOR_H
