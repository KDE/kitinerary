/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_EXTRACTORUTIL_H
#define KITINERARY_EXTRACTORUTIL_H

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

/** Try to extract postal codes included in the city name field.
 *  @internal Only exported for unit tests.
 */
KITINERARY_EXPORT PostalAddress extractPostalCode(PostalAddress addr);
}

}

#endif // KITINERARY_EXTRACTORUTIL_H