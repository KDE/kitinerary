/*
    SPDX-FileCopyrightText: ⓒ 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_UICSTATIONCODE_P_H
#define KITINERARY_UICSTATIONCODE_P_H

namespace KItinerary {

/** Utilities for UIC station codes. */
namespace UicStationCode
{
    /** Compute the 8th checksum digit for the given UIC station code. */
    [[nodiscard]] int checksumDigit(int uicCode);
}

}

#endif
