/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "fcbutil.h"

using namespace KItinerary;

QString FcbUtil::stringifyUicStationIdentifier(int num, const QByteArray &ia5)
{
    if (num >= 10'00000 && num <= 99'99999) {
        return QLatin1String("uic:") + QString::number(num);
    }
    if (ia5.size() == 7) {
        return QLatin1String("uic:") + QString::fromLatin1(ia5);
    }

    return {};
}

QString FcbUtil::stringifyStationIdentifier(bool numIsSet, int num, const QByteArray ia5)
{
    if (numIsSet) {
        return QString::number(num);
    }
    return QString::fromLatin1(ia5);
}
