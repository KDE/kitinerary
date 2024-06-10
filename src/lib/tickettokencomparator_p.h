/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_TICKETTOKENCOMPARATOR_H
#define KITINERARY_TICKETTOKENCOMPARATOR_H

#include "kitinerary_export.h"

class QVariant;

namespace KItinerary {

/** Compare ticket tokens for equivalence.
 *  For complex structured codes (UIC 918, IATA BCBP, etc) this is unfortunately
 *  a bit more complex than byte-wise equality.
 */
namespace TicketTokenComparator
{
    /** Compare QByteArray or QString ticket tokens.
     *  @internal exported for unit tests only
     */
    [[nodiscard]] KITINERARY_EXPORT bool isSame(const QVariant &lhs, const QVariant &rhs);
}

}

#endif // KITINERARY_TICKETTOKENCOMPARATOR_H
