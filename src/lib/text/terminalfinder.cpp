/*
    SPDX-FileCopyrightText: 2019-2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "terminalfinder_p.h"

#include <QDebug>

using namespace KItinerary;

struct {
    const char *pattern;
    QRegularExpression::PatternOptions options;
} static constexpr const terminal_patterns[] = {
    // use a named capture group for the actual name part of the terminal
    { " ?\\((?:terminal|aerogare) (?<name>\\w.*)\\)", QRegularExpression::CaseInsensitiveOption },
    { " ?\\((?<name>\\w.*) (?:terminal|aerogare)\\)", QRegularExpression::CaseInsensitiveOption },
    { "(?:, | ?- ?| )(?:terminal|aerogare) (?<name>\\w.*?)", QRegularExpression::CaseInsensitiveOption },
    { " T(?<name>\\d[A-Z]?)", QRegularExpression::NoPatternOption },
    { " ?\\(T(?<name>\\d[A-Z]?)\\)", QRegularExpression::NoPatternOption },
};


TerminalFinder::TerminalFinder(QStringView frontAnchor, QStringView backAnchor)
{
    static_assert(std::tuple_size_v<decltype(m_patterns)> == std::size(terminal_patterns));

    int i = 0;
    for (const auto &pattern : terminal_patterns) {
        m_patterns[i++] = QRegularExpression(frontAnchor + QLatin1String("(?<terminal>") + QLatin1String(pattern.pattern) + QLatin1String(")") + backAnchor, pattern.options);
    }
}

TerminalFinder::~TerminalFinder() = default;

TerminalFinder::Result TerminalFinder::find(QStringView s) const
{
    for (const auto &re: m_patterns) {
        const auto match = re.match(s);
        if (match.hasMatch()) {
            Result res;
            res.start = match.capturedStart(u"terminal");
            res.end = match.capturedEnd(u"terminal");
            res.name = match.captured(u"name"); // ### can't be a QStringView, that becomes invalid as soon as match goes out of scope (not when s becomes invalid!)
            return res;
        }
    }
    return {};
}

