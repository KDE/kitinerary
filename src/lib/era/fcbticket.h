/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_FCBTICKET_H
#define KITINERARY_FCBTICKET_H

#include "fcbticket1.h"
#include "fcbticket3.h"

#include <variant>

namespace KItinerary {

class UPERDecoder;

namespace Fcb {

using UicRailTicketData = std::variant<Fcb::v13::UicRailTicketData, Fcb::v3::UicRailTicketData>;

}
}

#endif
