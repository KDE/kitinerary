/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

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

