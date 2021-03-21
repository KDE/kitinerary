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
    const auto s = engine->barcodeDecoder()->decodeBinary(node.content<QImage>());
    if (!s.isEmpty()) {
        auto c = engine->documentNodeFactory()->createNode(s);
        node.appendChild(c);
    }
}
