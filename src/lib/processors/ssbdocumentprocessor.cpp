/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ssbdocumentprocessor.h"

#include "era/ssbticket.h"

#include <KItinerary/ExtractorFilter>

#include <QMetaProperty>

using namespace KItinerary;

bool SsbDocumentProcessor::canHandleData(const QByteArray &encodedData, [[maybe_unused]] QStringView fileName) const
{
    return SSBTicket::maybeSSB(encodedData);
}

ExtractorDocumentNode SsbDocumentProcessor::createNodeFromData(const QByteArray &encodedData) const
{
    ExtractorDocumentNode node;
    auto ticket = SSBTicket(encodedData);
    if (ticket.isValid()) {
        node.setContent(ticket);
    }
    return node;
}

bool SsbDocumentProcessor::matches(const ExtractorFilter &filter, const ExtractorDocumentNode &node) const
{
    const auto ticket = node.content<SSBTicket>();
    const auto propIdx = SSBTicket::staticMetaObject.indexOfProperty(filter.fieldName().toUtf8().constData());
    if (propIdx < 0) {
        return false;
    }
    const auto prop = SSBTicket::staticMetaObject.property(propIdx);
    const auto value = prop.readOnGadget(&ticket);
    return filter.matches(value.toString());
}
