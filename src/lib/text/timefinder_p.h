/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_TIMEFINDER_H
#define KITINERARY_TIMEFINDER_H

class QStringView;

#include <QVariant>

#include <vector>

namespace KItinerary {

/** Attempts to find time values in all locales mentioned in the given text. */
class TimeFinder
{
public:
    /** Search for all occurences of date/time values in @p text. */
    void find(QStringView text);

    /** Like the above, but assume there is exactly one time value in @p text.
     *  If anything else is found, returns an invalid QTime.
     */
    QTime findSingularTime(QStringView text);

    struct Result {
        int begin = -1;
        int end = -1;
        QVariant dateTime;
    };

    const std::vector<Result>& results() const;

private:
    void findTimes(QStringView text);
    void findDates(QStringView text);
    void mergeResults();

    std::vector<Result> m_results;
};

}

#endif // KITINERARY_TIMEFINDER_H
