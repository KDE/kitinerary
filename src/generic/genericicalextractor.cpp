/*
    Copyright (C) 2020 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
