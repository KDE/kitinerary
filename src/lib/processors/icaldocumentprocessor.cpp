/*
    SPDX-FileCopyrightText: 2020-2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "icaldocumentprocessor.h"
#include "logging.h"

#include <KItinerary/Event>
#include <KItinerary/ExtractorDocumentNodeFactory>
#include <KItinerary/ExtractorEngine>
#include <KItinerary/ExtractorFilter>
#include <KItinerary/ExtractorResult>
#include <KItinerary/Place>

#include <KCalendarCore/ICalFormat>
#include <KCalendarCore/MemoryCalendar>

#include <QJSEngine>
#include <QJSValue>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMetaProperty>
#include <QTimeZone>

#include <cstring>

using namespace KItinerary;

static bool contentStartsWith(const QByteArray &data, const char *str)
{
    auto it = data.begin();
    while (it != data.end() && std::isspace(static_cast<unsigned char>(*it))) {
        ++it;
    }

    const auto len = std::strlen(str);
    if ((int)len >= std::distance(it, data.end())) {
        return false;
    }
    return std::strncmp(it, str, len) == 0;
}

bool IcalCalendarProcessor::canHandleData(const QByteArray &encodedData, QStringView fileName) const
{
    return contentStartsWith(encodedData, "BEGIN:VCALENDAR")
        || fileName.endsWith(QLatin1String(".ics"), Qt::CaseInsensitive)
        || fileName.endsWith(QLatin1String(".ical"), Qt::CaseInsensitive);
}

ExtractorDocumentNode IcalCalendarProcessor::createNodeFromData(const QByteArray &encodedData) const
{
    KCalendarCore::Calendar::Ptr calendar(new KCalendarCore::MemoryCalendar(QTimeZone::systemTimeZone()));
    KCalendarCore::ICalFormat format;
    if (format.fromRawString(calendar, encodedData)) {
        calendar->setProductId(format.loadedProductId());
        ExtractorDocumentNode node;
        node.setContent(calendar);
        return node;
    } else {
        qCDebug(Log) << "Failed to parse iCal content.";
    }
    return {};
}

void IcalCalendarProcessor::expandNode(ExtractorDocumentNode &node, const ExtractorEngine *engine) const
{
    const auto cal = node.content<KCalendarCore::Calendar::Ptr>();
    for (const auto &event : cal->events()) {
        auto child = engine->documentNodeFactory()->createNode(QVariant::fromValue(event), u"internal/event");
        node.appendChild(child);
    }
}


bool IcalEventProcessor::matches(const ExtractorFilter &filter, const ExtractorDocumentNode &node) const
{
    const auto event = node.content<KCalendarCore::Event::Ptr>();
    return matchesGadget(filter, event.data());
}

void IcalEventProcessor::preExtract(ExtractorDocumentNode &node, [[maybe_unused]] const ExtractorEngine *engine) const
{
    const auto event = node.content<KCalendarCore::Event::Ptr>();
    const auto data = event->customProperty("KITINERARY", "RESERVATION");
    if (!data.isEmpty()) {
        node.addResult(QJsonDocument::fromJson(data.toUtf8()).array());
    }
}

void IcalEventProcessor::postExtract(ExtractorDocumentNode &node, const ExtractorEngine* engine) const
{
    if (!node.result().isEmpty() || (engine->hints() & ExtractorEngine::ExtractGenericIcalEvents) == 0) {
        return;
    }

    const auto event = node.content<KCalendarCore::Event::Ptr>();
    if (event->recurs() || event->hasRecurrenceId()) {
        return;
    }

    Event e;
    e.setName(event->summary());
    e.setDescription(event->description());
    e.setUrl(event->url());

    if (event->allDay()) {
        e.setStartDate(QDateTime(event->dtStart().date(), {0, 0}, Qt::LocalTime));
        e.setEndDate(QDateTime(event->dtEnd().date(), {23, 59, 59}, Qt::LocalTime));
    } else {
        e.setStartDate(event->dtStart());
        e.setEndDate(event->dtEnd());
    }

    Place venue;
    venue.setName(event->location()); // TODO attempt to detect addresses in here
    if (event->hasGeo()) {
        venue.setGeo({event->geoLatitude(), event->geoLongitude()});
    }
    e.setLocation(venue);

    // TODO attachments?

    node.addResult(QList<QVariant>{QVariant::fromValue(e)});
}

QJSValue IcalEventProcessor::contentToScriptValue(const ExtractorDocumentNode &node, QJSEngine *engine) const
{
    return engine->toScriptValue(*node.content<KCalendarCore::Event::Ptr>().data());
}
