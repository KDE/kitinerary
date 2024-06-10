/*
    SPDX-FileCopyrightText: 2018-2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "tickettokencomparator_p.h"
#include "compare-logging.h"

#include "uic9183/uic9183block.h"
#include "uic9183/uic9183header.h"
#include "uic9183/uic9183parser.h"

#include <cstring>

using namespace Qt::Literals::StringLiterals;
using namespace KItinerary;

/** Checks that @p lhs and @p rhs have a different prefix is they are both set. */
[[nodiscard]] static bool prefixConflictIfPresent(const QString &lhs, const QString &rhs, Qt::CaseSensitivity caseSensitive = Qt::CaseSensitive)
{
    return !lhs.isEmpty() && !rhs.isEmpty() && !lhs.startsWith(rhs, caseSensitive) && !rhs.startsWith(lhs, caseSensitive);
}
[[nodiscard]] static bool prefixConflictIfPresent(const QByteArray &lhs, const QByteArray &rhs)
{
    return !lhs.isEmpty() && !rhs.isEmpty() && !lhs.startsWith(rhs) && !rhs.startsWith(lhs);
}

[[nodiscard]] static bool isUicBlockSubset(const Uic9183Parser &lhs, const Uic9183Parser &rhs)
{
    for (auto block = lhs.firstBlock(); !block.isNull(); block = block.nextBlock()) {
        // ignore 0080VU blocks, those change in DB online tickets on every retrieval
        if (std::strncmp(block.name(), "0080VU", 6) == 0) {
            continue;
        }
        const auto rhsBlock = rhs.findBlock(block.name());
        if (rhsBlock.isNull() || rhsBlock != block) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] static bool compareUic918Token(const QByteArray &lhs, const QByteArray &rhs)
{
    if (!Uic9183Parser::maybeUic9183(lhs) || !Uic9183Parser::maybeUic9183(rhs)) {
        return false;
    }

    Uic9183Parser lhsUic;
    lhsUic.parse(lhs);
    Uic9183Parser rhsUic;
    rhsUic.parse(rhs);
    if (!lhsUic.isValid() || !rhsUic.isValid()) {
        return false;
    }

    return lhsUic.header() == rhsUic.header() && isUicBlockSubset(lhsUic, rhsUic) && isUicBlockSubset(rhsUic, lhsUic);
}

[[nodiscard]] static bool compareFlixbusToken(const QString &lhs, const QString &rhs)
{
    // Flixbus ticket tokens are URLs with varying host and query parts, while the path contains the actual token
    QUrl lhsUrl(lhs);
    QUrl rhsUrl(rhs);
    return lhsUrl.path() == rhsUrl.path();
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
        const auto lhsS = lhs.toString();
        const auto rhsS = rhs.toString();
        if (lhsS.contains("flixbus."_L1) && rhsS.contains("flixbus."_L1)) {
            return compareFlixbusToken(lhsS, rhsS);
        }

        return !prefixConflictIfPresent(lhsS, rhsS, Qt::CaseInsensitive);
    }
    if (lhs.userType() == QMetaType::QByteArray) {
        const auto lhsBA = lhs.toByteArray();
        const auto rhsBA = rhs.toByteArray();
        if (!prefixConflictIfPresent(lhsBA, rhsBA)) {
            return true;
        }

        return compareUic918Token(lhsBA, rhsBA);
    }

    qCWarning(CompareLog) << "unhandled ticket token type" << lhs << rhs;
    return false;
}
