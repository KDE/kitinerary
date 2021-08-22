/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "binarydocumentprocessor.h"

#include <KItinerary/ExtractorFilter>

#include <QQmlEngine>

#include <private/qv4arraybuffer_p.h>
#include <private/qv4engine_p.h>

using namespace KItinerary;

ExtractorDocumentNode BinaryDocumentProcessor::createNodeFromData(const QByteArray &encodedData) const
{
    ExtractorDocumentNode node;
    node.setContent(encodedData);
    return node;
}

bool BinaryDocumentProcessor::matches(const ExtractorFilter &filter, const ExtractorDocumentNode &node) const
{
    return filter.matches(QString::fromLatin1(node.content<QByteArray>()));
}

QJSValue BinaryDocumentProcessor::contentToScriptValue(const ExtractorDocumentNode &node, QJSEngine *engine) const
{
    return QJSValue(engine->handle(), engine->handle()->newArrayBuffer(node.content<QByteArray>())->asReturnedValue());
}
