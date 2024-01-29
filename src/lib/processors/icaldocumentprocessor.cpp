/*
    SPDX-FileCopyrightText: 2020-2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "icaldocumentprocessor.h"
#include "logging.h"
#include "stringutil.h"

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

bool IcalCalendarProcessor::canHandleData(const QByteArray &encodedData, QStringView fileName) const
{
  return StringUtil::startsWithIgnoreSpace(encodedData, "BEGIN:VCALENDAR") ||
         fileName.endsWith(QLatin1StringView(".ics"), Qt::CaseInsensitive) ||
         fileName.endsWith(QLatin1StringView(".ical"), Qt::CaseInsensitive);
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

void IcalEventProcessor::expandNode(ExtractorDocumentNode &node, const ExtractorEngine *engine) const
{
    const auto event = node.content<KCalendarCore::Event::Ptr>();
    const auto appleStructuredData = event->nonKDECustomProperty("X-APPLE-STRUCTURED-DATA");
    if (!appleStructuredData.isEmpty()) {
        auto child = engine->documentNodeFactory()->createNode(QByteArray::fromBase64(appleStructuredData.toLatin1()));
        node.appendChild(child);
    }
}

void IcalEventProcessor::preExtract(ExtractorDocumentNode &node, [[maybe_unused]] const ExtractorEngine *engine) const
{
    const auto event = node.content<KCalendarCore::Event::Ptr>();
    const auto data = event->customProperty("KITINERARY", "RESERVATION");
    if (!data.isEmpty()) {
        node.addResult(QJsonDocument::fromJson(data.toUtf8()).array());
    }

    if (!node.result().isEmpty() || event->recurs() || event->hasRecurrenceId()) {
        return;
    }

    Event e;
    e.setName(event->summary());
    e.setDescription(event->description());
    e.setUrl(event->url());

    if (event->allDay()) {
        e.setStartDate(QDateTime(event->dtStart().date(), {0, 0}, QTimeZone::LocalTime));
        e.setEndDate(QDateTime(event->dtEnd().date(), {23, 59, 59}, QTimeZone::LocalTime));
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

void IcalEventProcessor::postExtract(ExtractorDocumentNode &node, const ExtractorEngine* engine) const
{
    if ((engine->hints() & ExtractorEngine::ExtractGenericIcalEvents) || node.result().size() != 1 || !node.usedExtractor().isEmpty()) {
        return;
    }

    // remove the generic result again that we added above if no extractor script touched it
    if (JsonLd::isA<Event>(node.result().result().at(0))) {
        node.setResult({});
    }
}

QJSValue IcalEventProcessor::contentToScriptValue(const ExtractorDocumentNode &node, QJSEngine *engine) const
{
    return engine->toScriptValue(*node.content<KCalendarCore::Event::Ptr>().data());
}
