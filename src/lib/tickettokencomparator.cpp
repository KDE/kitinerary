/*
    SPDX-FileCopyrightText: 2018-2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "tickettokencomparator_p.h"
#include "compare-logging.h"

using namespace KItinerary;

/** Checks that @p lhs and @p rhs have a different prefix is they are both set. */
static bool prefixConflictIfPresent(const QString &lhs, const QString &rhs, Qt::CaseSensitivity caseSensitive = Qt::CaseSensitive)
{
    return !lhs.isEmpty() && !rhs.isEmpty() && !lhs.startsWith(rhs, caseSensitive) && !rhs.startsWith(lhs, caseSensitive);
}
static bool prefixConflictIfPresent(const QByteArray &lhs, const QByteArray &rhs)
{
    return !lhs.isEmpty() && !rhs.isEmpty() && !lhs.startsWith(rhs) && !rhs.startsWith(lhs);
}

bool KItinerary::TicketTokenComparator::isSame(const QVariant &lhs, const QVariant &rhs)
{
    if (lhs.isNull() || rhs.isNull()) {
        return true;
    }
    if (lhs.userType() != rhs.userType()) {
        return false;
    }

    if (lhs.userType() == QMetaType::QString) {
        return !prefixConflictIfPresent(lhs.toString(), rhs.toString(), Qt::CaseInsensitive);
    }
    if (lhs.userType() == QMetaType::QByteArray) {
        return !prefixConflictIfPresent(lhs.toByteArray(), rhs.toByteArray());
    }

    qCWarning(CompareLog) << "unhandled ticket token type" << lhs << rhs;
    return false;
}
