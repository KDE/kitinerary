/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "timefinder_p.h"

#include <QDebug>
#include <QRegularExpression>
#include <QStringView>
#include <QTime>

using namespace KItinerary;

static bool isSeparator(QChar c)
{
    return !c.isDigit() && !c.isLetter() && c != QLatin1Char(':');
}

void TimeFinder::find(QStringView text)
{
    m_times.clear();

    static const QRegularExpression rxTimes[] = {
        QRegularExpression(QStringLiteral("(?<hour>\\d?\\d)時(?<min>\\d\\d)分")),
        QRegularExpression(QStringLiteral("(?:(?<am>오전)|(?<pm>오후) ?)?(?<hour>\\d?\\d)시 ?(?<min>\\d?\\d)분")),
        QRegularExpression(QStringLiteral("(?:(?<am>上午)|(?<pm>下午))?(?<hour>\\d?\\d)點(?<min>\\d?\\d)分")),
        QRegularExpression(QStringLiteral("(?:(?<am>上午)|(?<pm>下午))(?<hour>\\d?\\d):(?<min>\\d?\\d)")),
        QRegularExpression(QStringLiteral("\\b(?<hour>\\d?\\d)[:h](?<min>\\d\\d)")),
        QRegularExpression(QStringLiteral("\\b(?<hour>\\d\\d)\\.(?<min>\\d\\d)(?=$|[^.])")),
    };
    static const QRegularExpression rxApSuffixes[] = {
        QRegularExpression(QStringLiteral("(?<pm> ?(?:pm|PM|p\\.m\\.|م|μ\\.μ\\.))")),
        QRegularExpression(QStringLiteral("(?<am> ?(?:am|AM|a\\.m\\.|ص|π\\.μ\\.))")),
        QRegularExpression(QStringLiteral("(?<pm>p)")),
        QRegularExpression(QStringLiteral("(?<am>a)")),
    };

    int rxTimesPattern = -1;
    for (auto i = 0; i < text.size(); ++i) {
        QRegularExpressionMatch rxTimeMatch;
        if (rxTimesPattern < 0) {
            rxTimesPattern = 0;
            for (const auto &rx : rxTimes) {
                rxTimeMatch = rx.match(text, i);
                if (rxTimeMatch.hasMatch()) {
                    break;
                }
                ++rxTimesPattern;
            }
        } else {
            rxTimeMatch = rxTimes[rxTimesPattern].match(text, i);
        }

        if (!rxTimeMatch.hasMatch()) {
            return;
        }

        i = rxTimeMatch.capturedEnd();
        QRegularExpressionMatch rxApMatch;
        for (const auto &rx : rxApSuffixes) {
            rxApMatch = rx.match(text, i, QRegularExpression::NormalMatch, QRegularExpression::AnchoredMatchOption);
            if (rxApMatch.hasMatch()) {
                break;
            }
        }
        if (rxApMatch.hasMatch()) {
            i = rxApMatch.capturedEnd();
        }

        if (i < text.size() && !isSeparator(text[i])) {
            continue;
        }

        auto hour = rxTimeMatch.captured(u"hour").toInt();
        const auto min = rxTimeMatch.captured(u"min").toInt();
        if (hour < 0 || hour > 23 || min < 0 || min > 59) {
            continue;
        }
        const bool isPm = !rxApMatch.captured(u"pm").isEmpty() || !rxTimeMatch.captured(u"pm").isEmpty();
        const bool isAm = !rxApMatch.captured(u"am").isEmpty() || !rxTimeMatch.captured(u"am").isEmpty();
        if (isPm && isAm) {
            continue;
        } else if (isPm && hour < 12) {
            hour += 12;
        } else if (isAm && hour == 12) {
            hour = 0;
        }

        if (std::find(m_times.begin(), m_times.end(), QTime(hour, min)) == m_times.end()) {
            m_times.push_back(QTime(hour, min));
        }
    }
}

const std::vector<QTime>& TimeFinder::times() const
{
    return m_times;
}
