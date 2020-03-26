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
#include <vector>

class QMetaObject;
class QVariant;

namespace KItinerary {
class ExtractorValidatorPrivate;

/**
 * Validates extractor results.
 * Used to discard incomplete or otherwise invalid data.
 * @since 20.08
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

    /** Sets the list of supported top-level types that should be accepted.
     *  Providing an empty set of types will accept all top-level types.
     *  Instances of types inheriting from accepted types are also accepted.
     *  Default is to accept all types.
     */
    void setAcceptedTypes(std::vector<const QMetaObject*> &&accptedTypes);
    /** Convenience overload of setAcceptedTypes(). */
    template <typename ...Args> inline void setAcceptedTypes()
    {
        setAcceptedTypes({&Args::staticMetaObject...});
    }

private:
    std::unique_ptr<ExtractorValidatorPrivate> d;
};

}

#endif // EXTRACTORVALIDATOR_H
