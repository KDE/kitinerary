/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "binarydocumentprocessor.h"

#include <KItinerary/ExtractorFilter>

#include <QQmlEngine>

using namespace KItinerary;

bool BinaryDocumentProcessor::canHandleData([[maybe_unused]] const QByteArray &data, [[maybe_unused]] QStringView fileName) const
{
    return true;
}

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
    return QJSValue(QJSManagedValue(QVariant(node.content<QByteArray>()), engine));
}
