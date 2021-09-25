/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_TIMEFINDER_H
#define KITINERARY_TIMEFINDER_H

class QStringView;
class QTime;

#include <vector>

namespace KItinerary {

/** Attempts to find time values in all locales mentioned in the given text. */
class TimeFinder
{
public:
    void find(QStringView text);

    const std::vector<QTime>& times() const;

private:
    std::vector<QTime> m_times;
};

}

#endif // KITINERARY_TIMEFINDER_H
