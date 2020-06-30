/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <config-kitinerary.h>

#include "genericicalextractor_p.h"
#include "genericextractor_p.h"

#ifdef HAVE_KCAL
#include <KCalendarCore/Calendar>
#include <KCalendarCore/Event>

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>

using namespace KItinerary;

static GenericExtractor::Result extractEvent(const KCalendarCore::Event::Ptr &event)
{
    const auto data = event->customProperty("KITINERARY", "RESERVATION");
    if (data.isEmpty()) {
        return {};
    }
    const auto array = QJsonDocument::fromJson(data.toUtf8()).array();
    if (array.isEmpty()) {
        return {};
    }
    return GenericExtractor::Result(array);
}

std::vector<GenericExtractor::Result> GenericIcalExtractor::extract(const KCalendarCore::Calendar::Ptr &calendar)
{
    std::vector<GenericExtractor::Result> results;
    for (const auto &event : calendar->events()) {
        auto res = extractEvent(event);
        if (!res.isEmpty()) {
            results.push_back(std::move(res));
        }
    }
    return results;
}

#endif
