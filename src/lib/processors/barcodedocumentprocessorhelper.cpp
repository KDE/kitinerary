/*
    SPDX-FileCopyrightText: 2021-2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "barcodedocumentprocessorhelper.h"

#include <KItinerary/ExtractorDocumentNode>
#include <KItinerary/ExtractorDocumentNodeFactory>
#include <KItinerary/ExtractorEngine>

using namespace KItinerary;

void BarcodeDocumentProcessorHelper::expandNode(const QImage &img, BarcodeDecoder::BarcodeTypes barcodeHints, ExtractorDocumentNode &parent, const ExtractorEngine* engine)
{
    // in case the barcode raw data (string or bytearray) gets detected as a type we handle,
    // we nevertheless inject a raw data node in between. This is useful in cases where the
    // content is parsable but that is actually not desired (e.g. JSON content in ticket barcodes).

    const auto b = engine->barcodeDecoder()->decodeBinary(img, barcodeHints);
    if (!b.isEmpty()) {
        auto c = engine->documentNodeFactory()->createNode(b);
        if (c.isA<QByteArray>() || c.isA<QString>()) {
            parent.appendChild(c);
            return;
        }
        auto rawNode = engine->documentNodeFactory()->createNode(QVariant::fromValue(b), u"application/octet-stream");
        rawNode.appendChild(c);
        parent.appendChild(rawNode);
        return;
    }

    const auto s = engine->barcodeDecoder()->decodeString(img, barcodeHints);
    if (!s.isEmpty()) {
        auto c = engine->documentNodeFactory()->createNode(s.toUtf8());
        if (c.isA<QByteArray>() || c.isA<QString>()) {
            parent.appendChild(c);
            return;
        }
        auto rawNode = engine->documentNodeFactory()->createNode(QVariant::fromValue(s), u"text/plain");
        rawNode.appendChild(c);
        parent.appendChild(rawNode);
        return;
    }
}
