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

#ifndef KITINERARY_EXTRACTORVALIDATOR_H
#define KITINERARY_EXTRACTORVALIDATOR_H

#include "kitinerary_export.h"

#include <memory>

class QVariant;

namespace KItinerary {
class ExtractorValidatorPrivate;

/**
 * Validates extractor results.
 * Used to discard incomplete or otherwise invalid data.
 */
class KITINERARY_EXPORT ExtractorValidator
{
public:
    ExtractorValidator();
    ~ExtractorValidator();
    ExtractorValidator(const ExtractorValidator&) = delete;
    ExtractorValidator& operator=(const ExtractorValidator&) = delete;

    /** Checks if the given element is valid.
     *  This will accept both Reservation object and things
     *  that can be reserved as top-level objects.
     */
    bool isValidElement(const QVariant &elem);

private:
    std::unique_ptr<ExtractorValidatorPrivate> d;
};

}

#endif // EXTRACTORVALIDATOR_H
