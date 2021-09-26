/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "timefinder_p.h"

#include <QDebug>
#include <QLocale>
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
    findTimes(text);
    if (!m_results.empty()) {
        findDates(text);
        mergeResults();
    }

    for (const auto &res : m_results) {
        qDebug() << "  " << res.dateTime << res.begin << res.end;
    }
}

void TimeFinder::findTimes(QStringView text)
{
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

        auto hour = rxTimeMatch.capturedView(u"hour").toInt();
        const auto min = rxTimeMatch.capturedView(u"min").toInt();
        if (hour < 0 || hour > 23 || min < 0 || min > 59) {
            continue;
        }
        const bool isPm = !rxApMatch.capturedView(u"pm").isEmpty() || !rxTimeMatch.capturedView(u"pm").isEmpty();
        const bool isAm = !rxApMatch.capturedView(u"am").isEmpty() || !rxTimeMatch.capturedView(u"am").isEmpty();
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
        Result result;
        result.dateTime = QTime(hour, min);
        result.begin = rxTimeMatch.capturedStart();
        result.end = i;
        m_results.push_back(std::move(result));
    }
}

static int monthToNumber(QStringView month)
{
    bool result = false;
    auto num = month.toInt(&result);
    if (result) {
        return num;
    }

    for (int i = 1; i <= 12; ++i) {
        if (QLocale::c().monthName(i, QLocale::ShortFormat).compare(month, Qt::CaseInsensitive) == 0) {
            return i;
        }
    }
    return 0;
}

void TimeFinder::findDates(QStringView text)
{
    // ### unlike times, this is far from complete and is only here to the extend necessary for
    // detecting generation timestamps we don't want to consider in boarding passes
    static const QRegularExpression rxDates[] = {
        QRegularExpression(QStringLiteral("(?<day>\\d\\d)\\.(?<mon>\\d\\d)\\.(?<year>\\d{4})")),
        QRegularExpression(QStringLiteral("(?<day>\\d\\d) (?<mon>[A-Z][a-zA-Z]{2}) (?<year>\\d{4})")),
    };

    for (const auto &rx : rxDates) {
        for (int idx = 0; idx < text.size(); ++idx) {
            const auto rxDateMatch = rx.match(text, idx);
            if (!rxDateMatch.hasMatch()) {
                break;
            }

            idx = rxDateMatch.capturedEnd();
            const auto day = rxDateMatch.capturedView(u"day").toInt();
            const auto month = monthToNumber(rxDateMatch.capturedView(u"mon"));
            const auto year = rxDateMatch.capturedView(u"year").toInt();
            QDate date(year, month, day);
            if (!date.isValid()) {
                continue;
            }

            Result result;
            result.dateTime = date;
            result.begin = rxDateMatch.capturedStart();
            result.end = idx;
            m_results.push_back(std::move(result));
        }
    }
}

void TimeFinder::mergeResults()
{
    std::sort(m_results.begin(), m_results.end(), [](const auto &lhs, const auto &rhs) {
        return lhs.begin < rhs.begin;
    });

    for (auto it = m_results.begin(); it != m_results.end() && it != std::prev(m_results.end());) {
        auto nextIt = std::next(it);
        if ((*it).end + 1 == (*nextIt).begin && (*it).dateTime.type() == QVariant::Date && (*nextIt).dateTime.type() == QVariant::Time) {
            (*it).end = (*nextIt).end;
            (*it).dateTime = QDateTime((*it).dateTime.toDate(), (*nextIt).dateTime.toTime());
            it = m_results.erase(nextIt);
        } else {
            ++it;
        }
    }
}

const std::vector<QTime>& TimeFinder::times() const
{
    return m_times;
}

const std::vector<TimeFinder::Result>& TimeFinder::results() const
{
    return m_results;
}
