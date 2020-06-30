/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
    bool isValidElement(const QVariant &elem) const;

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

    /** Configure whether or not to accept also incomplete elements.
     *  The default is @c true.
     *  Accepting incomplete elements is useful if the output is
     *  further processed, for example to merge minimal cancellation
     *  elements with already existing data. If the output is displayed
     *  directly, set this to @c true.
     */
    void setAcceptOnlyCompleteElements(bool completeOnly);

private:
    std::unique_ptr<ExtractorValidatorPrivate> d;
};

}

#endif // EXTRACTORVALIDATOR_H
