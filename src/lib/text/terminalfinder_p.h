/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_TERMINALFINDER_P_H
#define KITINERARY_TERMINALFINDER_P_H

#include <QRegularExpression>

#include <array>

class QStringView;

namespace KItinerary {

/** Identify terminal names alongside airport names. */
class TerminalFinder
{
public:
    /** Create a terminal finder with custom front and back anchor patterns. */
    explicit TerminalFinder(QStringView frontAnchor, QStringView backAnchor);
    ~TerminalFinder();

    struct Result {
        // start and end offsets of the terminal part (not including front/back anchor patterns)
        int start = -1;
        int end = -1;
        QString name;

        constexpr inline bool hasResult() const { return start >= 0; }
    };

    Result find(QStringView s) const;

private:
    std::array<QRegularExpression, 5> m_patterns;
};

}

#endif // KITINERARY_TERMINALFINDER_P_H
