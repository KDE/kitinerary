/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_EXTRACTORCAPABILITIES_H
#define KITINERARY_EXTRACTORCAPABILITIES_H

#include "kitinerary_export.h"

class QString;

namespace KItinerary {

/** Diagnositic information about which features of the extractor are available.
 *  This typically depends on build options.
 */
namespace ExtractorCapabilities
{
    /** Textual representation, mainly useful for bug reports/support. */
    KITINERARY_EXPORT QString capabilitiesString();
}

}

#endif // KITINERARY_EXTRACTORCAPABILITIES_H
