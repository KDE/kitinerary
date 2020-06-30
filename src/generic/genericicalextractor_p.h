/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_GENERICICALEXTRACTOR_P_H
#define KITINERARY_GENERICICALEXTRACTOR_P_H

#include <QSharedPointer>

#include <vector>

namespace KCalendarCore {
class Calendar;
}

namespace KItinerary {

namespace GenericExtractor {
class Result;
}

/** Generic extraction from ical calendars/events. */
namespace GenericIcalExtractor
{
    std::vector<GenericExtractor::Result> extract(const QSharedPointer<KCalendarCore::Calendar> &calendar);
}

}

#endif // KITINERARY_GENERICICALEXTRACTOR_H
