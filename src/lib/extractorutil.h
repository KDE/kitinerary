/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kitinerary_export.h"

namespace KItinerary {

class Flight;
class PostalAddress;

/** Data extraction utility functions. */
namespace ExtractorUtil
{
/** Move terminal indications from airport names to the correct fields.
 *  @internal Only exported for unit tests.
 */
KITINERARY_EXPORT Flight extractTerminals(Flight flight);
}

}

