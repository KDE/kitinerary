/*
    SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "dosipasfactory_p.h"

using namespace KItinerary;

QVariant DosipasFactory::decodeDataType(QByteArrayView format, const QByteArray &data)
{
    if (auto fcb = decodeFcb(format, data); fcb) {
        return std::visit([](auto &&fcb) { return QVariant::fromValue(fcb); }, *fcb);
    }

    // TODO FCD, vendor extensions

    return {};
}

std::optional<Fcb::UicRailTicketData> DosipasFactory::decodeFcb(QByteArrayView format, const QByteArray &data)
{
    if (format == "FCB3") {
        auto fcb = Fcb::v3::UicRailTicketData(data);
        return fcb.isValid() ? std::optional<Fcb::UicRailTicketData>(fcb) : std::nullopt;
    }
    if (format == "FCB1") {
        auto fcb = Fcb::v13::UicRailTicketData(data);
        return fcb.isValid() ? std::optional<Fcb::UicRailTicketData>(fcb) : std::nullopt;
    }

    return {};
}
