/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "imagedocumentprocessor.h"
#include "barcodedocumentprocessorhelper.h"

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
    // skip barcode extraction if somebody else already did that (such as the PDF processor)
    if (!node.childNodes().empty()) {
        return;
    }

    const auto img = node.content<QImage>();

    BarcodeDecoder::BarcodeTypes barcodeHints = BarcodeDecoder::Any2D;
    // check whether we possibly have a full PDF page raster image here
    if (engine->hints() & ExtractorEngine::ExtractFullPageRasterImages) {
        barcodeHints |= BarcodeDecoder::IgnoreAspectRatio;
    }
    barcodeHints = BarcodeDecoder::maybeBarcode(img.width(), img.height(), barcodeHints);

    BarcodeDocumentProcessorHelper::expandNode(img, barcodeHints, node, engine);
}
