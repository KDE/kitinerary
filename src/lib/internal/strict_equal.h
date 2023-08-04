/*
    SPDX-FileCopyrightText: 2018-2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_INTERNAL_STRICT_EQUAL_H
#define KITINERARY_INTERNAL_STRICT_EQUAL_H

#include "parameter_type.h"

#include <QDateTime>
#include <QString>
#include <QTimeZone>
#include <QVariant>

#include <cmath>

namespace KItinerary {
namespace detail {

/** Stricter than operator== equality comparison.
 *  For some types operator== implements a weaker form of "semantic" equality,
 *  not exact equality of the value itself.
 */

template <typename T>
inline bool strict_equal(typename parameter_type<T>::type lhs, typename parameter_type<T>::type rhs)
{
    return lhs == rhs;
}

// compare QVariant contents (no longer the default with Qt6)
template <> inline
bool strict_equal<QVariant>(const QVariant &lhs, const QVariant &rhs)
{
    return lhs.isNull() == rhs.isNull() && (lhs.isNull() || QVariant::compare(lhs, rhs) == QPartialOrdering::Equivalent);
}

// QDateTime::operator== is true for two instances referring to the same point in time
// we however want to know if two instances contain exactly the same information
template <>
inline bool strict_equal<QDateTime>(const QDateTime &lhs, const QDateTime &rhs)
{
    if (lhs.timeSpec() != rhs.timeSpec() || lhs != rhs) {
        return false;
    }
    return lhs.timeSpec() == Qt::TimeZone ? lhs.timeZone() == rhs.timeZone() : true;
}

// QString::operator== ignores null vs empty
// we probably don't care either, but until that's decided this makes the existing tests pass
template <>
inline bool strict_equal<QString>(const QString &lhs, const QString &rhs)
{
    if (lhs.isEmpty() && rhs.isEmpty()) {
        return lhs.isNull() == rhs.isNull();
    }
    return lhs == rhs;
}

// Floating point numbers: we treat two NAN values as equal
template <>
inline bool strict_equal<float>(float lhs, float rhs)
{
    return lhs == rhs || (std::isnan(lhs) && std::isnan(rhs));
}
template <>
inline bool strict_equal<double>(double lhs, double rhs)
{
    return lhs == rhs || (std::isnan(lhs) && std::isnan(rhs));
}

}
}

#endif

