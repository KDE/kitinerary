/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_IATABCBPCONSTANTS_H
#define KITINERARY_IATABCBPCONSTANTS_H

namespace KItinerary {
namespace IataBcbpConstants {
enum {
    UniqueMandatorySize = 23,
    RepeatedMandatorySize = 37,
    RepeatedMandatoryMinimalViableSize = 24, // pre-checkin data, technically incomplete, but we can make use of this nevertheless
    MinimumViableSize = UniqueMandatorySize + RepeatedMandatoryMinimalViableSize,
    MinimumUniqueConditionalSize = 4,
    MinimumSecuritySectionSize = 4,
};
}
}

#endif
