/*
    SPDX-FileCopyrightText: 2021-2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "barcodedocumentprocessorhelper.h"

#include <KItinerary/ExtractorDocumentNode>
#include <KItinerary/ExtractorDocumentNodeFactory>
#include <KItinerary/ExtractorEngine>

using namespace KItinerary;

static bool appendBarcodeResult(const BarcodeDecoder::Result &result, ExtractorDocumentNode &parent, const ExtractorEngine *engine)
{
    if (result.contentType == BarcodeDecoder::Result::None) {
        return false;
    }

    ExtractorDocumentNode childNode;
    if (result.contentType & BarcodeDecoder::Result::ByteArray) {
        childNode = engine->documentNodeFactory()->createNode(result.toByteArray());
    } else {
        childNode = engine->documentNodeFactory()->createNode(result.toString().toUtf8());
    }

    // in case the barcode raw data (string or bytearray) gets detected as a type we handle,
    // we nevertheless inject a raw data node in between. This is useful in cases where the
    // content is parsable but that is actually not desired (e.g. JSON content in ticket barcodes).

    if (childNode.isA<QByteArray>() || childNode.isA<QString>()) {
        parent.appendChild(childNode);
        return true;
    }

    ExtractorDocumentNode rawNode;
    if (result.contentType & BarcodeDecoder::Result::ByteArray) {
        rawNode = engine->documentNodeFactory()->createNode(result.content, u"application/octet-stream");
    } else {
        rawNode = engine->documentNodeFactory()->createNode(result.content, u"text/plain");
    }
    rawNode.appendChild(childNode);
    parent.appendChild(rawNode);
    return true;
}

bool BarcodeDocumentProcessorHelper::expandNode(const QImage &img, BarcodeDecoder::BarcodeTypes barcodeHints, ExtractorDocumentNode &parent, const ExtractorEngine* engine)
{
    if (barcodeHints & BarcodeDecoder::IgnoreAspectRatio) {
        const auto results = engine->barcodeDecoder()->decodeMulti(img, barcodeHints);
        bool found = false;
        for (const auto &res : results) {
            found = appendBarcodeResult(res, parent, engine) || found; // no short-circuit evaluation!
        }
        return found;
    } else {
        return appendBarcodeResult(engine->barcodeDecoder()->decode(img, barcodeHints), parent, engine);
    }
}
