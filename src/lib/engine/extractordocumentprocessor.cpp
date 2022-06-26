/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "extractordocumentprocessor.h"
#include "extractorfilter.h"
#include "extractorresult.h"
#include "logging.h"

#include <QJSEngine>
#include <QJSValue>
#include <QMetaProperty>

using namespace KItinerary;

ExtractorDocumentProcessor::~ExtractorDocumentProcessor() = default;

bool ExtractorDocumentProcessor::canHandleData([[maybe_unused]] const QByteArray &encodedData, [[maybe_unused]] QStringView fileName) const
{
    return false;
}

ExtractorDocumentNode ExtractorDocumentProcessor::createNodeFromData([[maybe_unused]] const QByteArray &encodedData) const
{
    return {};
}

ExtractorDocumentNode ExtractorDocumentProcessor::createNodeFromContent(const QVariant &decodedData) const
{
    ExtractorDocumentNode node;
    node.setContent(decodedData);
    return node;
}

void ExtractorDocumentProcessor::expandNode([[maybe_unused]] ExtractorDocumentNode &node, [[maybe_unused]] const ExtractorEngine *engine) const
{
}

void ExtractorDocumentProcessor::reduceNode(ExtractorDocumentNode &node) const
{
    for (const auto &child : node.childNodes()) {
        node.addResult(child.result());
    }
}

void ExtractorDocumentProcessor::preExtract([[maybe_unused]] ExtractorDocumentNode &node, [[maybe_unused]] const ExtractorEngine *engine) const
{
}

bool ExtractorDocumentProcessor::matches(const ExtractorFilter &filter, const ExtractorDocumentNode &node) const
{
    // QObject content
    if (node.content().canConvert<QObject*>()) {
        const auto obj = node.content().value<QObject*>();
        if (!obj) {
            return false;
        }
        const auto value = obj->property(filter.fieldName().toUtf8().constData());
        return filter.matches(value.toString());
    }

    // Q_GADGET content
    const auto mo = QMetaType(node.content().userType()).metaObject();
    return matchesGadget(filter, mo, node.content().constData());
}

static bool matchesGadgetInternal(const ExtractorFilter &filter, QStringView propName, const QMetaObject *mo, const void *obj)
{
    if (!mo) {
        return false;
    }

    const auto propNameIdx = propName.indexOf(QLatin1Char('.'));
    if (propNameIdx == 0 || propName.isEmpty()) {
        qCWarning(Log) << "invalid gadget property name:" << propName << filter.fieldName();
        return false;
    }

    const auto propIdx = mo->indexOfProperty(propName.left(propNameIdx < 0 ? propName.size() : propNameIdx).toUtf8().constData());
    if (propIdx < 0) {
        return false;
    }
    const auto prop = mo->property(propIdx);
    const auto value = prop.readOnGadget(obj);

    if (propNameIdx > 0) { // recursive matching
        const auto mo = QMetaType(value.userType()).metaObject();
        return matchesGadgetInternal(filter, propName.mid(propNameIdx + 1), mo, value.constData());
    }
    return filter.matches(value.toString());
}

bool ExtractorDocumentProcessor::matchesGadget(const ExtractorFilter &filter, const QMetaObject *mo, const void *obj)
{
    return matchesGadgetInternal(filter, filter.fieldName(), mo, obj);
}

void ExtractorDocumentProcessor::postExtract([[maybe_unused]] ExtractorDocumentNode &node) const
{
}

QJSValue ExtractorDocumentProcessor::contentToScriptValue([[maybe_unused]] const ExtractorDocumentNode &node, QJSEngine *engine) const
{
    return engine->toScriptValue(node.content());
}

void ExtractorDocumentProcessor::destroyNode([[maybe_unused]] ExtractorDocumentNode &node) const
{
}
