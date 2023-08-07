/*
    SPDX-FileCopyrightText: 2020-2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KItinerary/ExtractorDocumentProcessor>

namespace KItinerary {

/** Processor for ical calendars. */
class IcalCalendarProcessor : public ExtractorDocumentProcessor
{
public:
    bool canHandleData(const QByteArray &encodedData, QStringView fileName) const override;
    ExtractorDocumentNode createNodeFromData(const QByteArray &encodedData) const override;
    void expandNode(ExtractorDocumentNode &node, const ExtractorEngine *engine) const override;
};

/** Processor for ical calendar events. */
class IcalEventProcessor : public ExtractorDocumentProcessor
{
public:
    bool matches(const ExtractorFilter &filter, const ExtractorDocumentNode &node) const override;
    void expandNode(ExtractorDocumentNode &node, const ExtractorEngine *engine) const override;
    void preExtract(ExtractorDocumentNode &node, const ExtractorEngine *engine) const override;
    void postExtract(ExtractorDocumentNode &node, const ExtractorEngine *engine) const override;
    QJSValue contentToScriptValue(const ExtractorDocumentNode &node, QJSEngine *engine) const override;
};

}
