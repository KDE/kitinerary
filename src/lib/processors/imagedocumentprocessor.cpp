/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "imagedocumentprocessor.h"

#include <KItinerary/BarcodeDecoder>
#include <KItinerary/ExtractorDocumentNodeFactory>
#include <KItinerary/ExtractorEngine>

#include <QImage>

using namespace KItinerary;

void ImageDocumentProcessor::expandNode(ExtractorDocumentNode &node, const ExtractorEngine *engine) const
{
    // in case the barcode raw data (string or bytearray) gets detected as a type we handle,
    // we nevertheless inject a raw data node inbetween. This is useful in cases where the
    // content is parsable but that is actually not desired (e.g. JSON content in ticket barcodes).

    const auto b = engine->barcodeDecoder()->decodeBinary(node.content<QImage>());
    if (!b.isEmpty()) {
        auto c = engine->documentNodeFactory()->createNode(b);
        if (c.isA<QByteArray>() || c.isA<QString>()) {
            node.appendChild(c);
            return;
        }
        auto rawNode = engine->documentNodeFactory()->createNode(QVariant::fromValue(b), u"application/octet-stream");
        rawNode.appendChild(c);
        node.appendChild(rawNode);
        return;
    }

    const auto s = engine->barcodeDecoder()->decodeString(node.content<QImage>());
    if (!s.isEmpty()) {
        auto c = engine->documentNodeFactory()->createNode(s.toUtf8());
        if (c.isA<QByteArray>() || c.isA<QString>()) {
            node.appendChild(c);
            return;
        }
        auto rawNode = engine->documentNodeFactory()->createNode(QVariant::fromValue(s), u"text/plain");
        rawNode.appendChild(c);
        node.appendChild(rawNode);
        return;
    }
}
