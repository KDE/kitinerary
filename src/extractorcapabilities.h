/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
