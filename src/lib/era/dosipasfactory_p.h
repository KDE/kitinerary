/*
    SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_DOSIPASFACTORY_H
#define KITINERARY_DOSIPASFACTORY_H

#include "fcbticket.h"

#include <QVariant>

namespace KItinerary {

/** Create specific objects for DOSIPAS payloads. */
class DosipasFactory
{
public:
    template <typename T>
    [[nodiscard]] static inline QVariant decodeDataType(const T &dataType)
    {
        return decodeDataType(dataType.dataFormat, dataType.data);
    }

    template <typename T>
    [[nodiscard]] static inline std::optional<Fcb::UicRailTicketData> decodeFcb(const T &dataType)
    {
        return decodeFcb(dataType.dataFormat, dataType.data);
    }

private:
    [[nodiscard]] static QVariant decodeDataType(QByteArrayView format, const QByteArray &data);
    [[nodiscard]] static std::optional<Fcb::UicRailTicketData> decodeFcb(QByteArrayView format, const QByteArray &data);

};

}

#endif
