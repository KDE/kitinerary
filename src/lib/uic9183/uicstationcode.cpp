/*
    SPDX-FileCopyrightText: ⓒ 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "uicstationcode_p.h"

#include <QDebug>

using namespace KItinerary;

int UicStationCode::checksumDigit(int uicCode)
{
    if (uicCode > 99'99999 || uicCode < 10'00000) {
        return -1;
    }

    int input = uicCode % 100000;
    int sum = 0;
    for (int i = 0; i < 5; ++i) {
        const int k = (2 - (i % 2)) * (input % 10);
        if (k >= 10) {
            sum += (k / 10) + (k % 10);
        } else {
            sum += k;
        }
        input /= 10;
    }
    return (10 - (sum % 10)) % 10;
}
