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

ExtractorDocumentNode ImageDocumentProcessor::createNodeFromData(const QByteArray &encodedData) const
{
    const auto img = QImage::fromData(encodedData);
    if (img.isNull()) {
        return {};
    }

    ExtractorDocumentNode node;
    node.setContent(img);
    return node;
}

void ImageDocumentProcessor::expandNode(ExtractorDocumentNode &node, const ExtractorEngine *engine) const
{
    // check whether we possibly have a full PDF page raster image here
    const auto img = node.content<QImage>();
    BarcodeDecoder::BarcodeTypes barcodeHints = BarcodeDecoder::Any2D;
    if (engine->hints() & ExtractorEngine::ExtractFullPageRasterImages && !BarcodeDecoder::maybeBarcode(img.width(), img.height(), barcodeHints)) {
        barcodeHints |= BarcodeDecoder::IgnoreAspectRatio;
    }

    // in case the barcode raw data (string or bytearray) gets detected as a type we handle,
    // we nevertheless inject a raw data node in between. This is useful in cases where the
    // content is parsable but that is actually not desired (e.g. JSON content in ticket barcodes).

    const auto b = engine->barcodeDecoder()->decodeBinary(img, barcodeHints);
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

    const auto s = engine->barcodeDecoder()->decodeString(img, barcodeHints);
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
