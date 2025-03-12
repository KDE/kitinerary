/*
    SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "dosipasfactory_p.h"

#include "fcbticket.h"
#include "fcbticket3.h"

using namespace KItinerary;

QVariant DosipasFactory::decodeDataType(QByteArrayView format, const QByteArray &data)
{
    if (format == "FCB3") {
        return QVariant::fromValue(Fcb::v3::UicRailTicketData(data));
    }
    if (format == "FCB1") {
        return QVariant::fromValue(Fcb::v13::UicRailTicketData(data));
    }

    return {};
}
