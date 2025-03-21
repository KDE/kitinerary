/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_FCBTICKET_H
#define KITINERARY_FCBTICKET_H

#include "fcbticket1.h"
#include "fcbticket2.h"
#include "fcbticket3.h"

#include <variant>

#define FCB_VERSIONED(T) \
    Fcb::v13::T, Fcb::v2::T, Fcb::v3::T

namespace KItinerary {

class UPERDecoder;

namespace Fcb {

using UicRailTicketData = std::variant<FCB_VERSIONED(UicRailTicketData)>;


}
}

#endif
