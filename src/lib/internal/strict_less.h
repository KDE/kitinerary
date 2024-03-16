/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_INTERNAL_STRICT_LESS_H
#define KITINERARY_INTERNAL_STRICT_LESS_H

#include "parameter_type.h"

#include <QDateTime>
#include <QVariant>

namespace KItinerary {
namespace detail {

/** Less-than counterpart to strict_equal.h
 *  This is far less complete though, as we only need this to make equality comparison
 *  work in Qt6, which requires a less-than comparison to be present.
 */

template <typename T>
inline bool strict_less(typename parameter_type<T>::type lhs, typename parameter_type<T>::type rhs)
{
    return lhs < rhs;
}

template <> inline
bool strict_less<QDateTime>(const QDateTime &lhs, const QDateTime &rhs)
{
    if (lhs == rhs) {
        return lhs.timeSpec() < rhs.timeSpec();
    }
    return lhs < rhs;
}

// compare QVariant contents (no longer the default with Qt6)
template <> inline
bool strict_less<QVariant>(const QVariant &lhs, const QVariant &rhs)
{
    return !lhs.isNull() && !rhs.isNull() && QVariant::compare(lhs, rhs) == QPartialOrdering::Less;
}

template<> inline
bool strict_less<QList<QVariant>>([[maybe_unused]] const QList<QVariant> &lhs, [[maybe_unused]] const QList<QVariant> &rhs)
{
    return false;
}

}
}

#endif

