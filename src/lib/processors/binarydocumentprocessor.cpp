/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "binarydocumentprocessor.h"

#include <KItinerary/ExtractorFilter>

#include <QQmlEngine>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <private/qv4arraybuffer_p.h>
#include <private/qv4engine_p.h>
#endif

using namespace KItinerary;

ExtractorDocumentNode BinaryDocumentProcessor::createNodeFromData(const QByteArray &encodedData) const
{
    ExtractorDocumentNode node;
    node.setContent(encodedData);
    return node;
}

bool BinaryDocumentProcessor::matches(const ExtractorFilter &filter, const ExtractorDocumentNode &node) const
{
    const auto b = node.content<QByteArray>();
    return filter.matches(QString::fromLatin1(b.constData(), b.size()));
}

QJSValue BinaryDocumentProcessor::contentToScriptValue(const ExtractorDocumentNode &node, QJSEngine *engine) const
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    return QJSValue(engine->handle(), engine->handle()->newArrayBuffer(node.content<QByteArray>())->asReturnedValue());
#else
    return QJSValue(QJSManagedValue(QVariant(node.content<QByteArray>()), engine));
#endif
}
