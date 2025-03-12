/*
    SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "dosipasdocumentprocessor.h"

#include "era/dosipas1.h"
#include "era/dosipas2.h"

using namespace KItinerary;

bool DosipasDocumentProcessor::canHandleData(const QByteArray &encodedData, [[maybe_unused]] QStringView fileName) const
{
    return encodedData.startsWith("\x01Uc") || encodedData.startsWith("\x01Ue")
        || encodedData.startsWith("\x81Uc") || encodedData.startsWith("\x81Ue");
}

ExtractorDocumentNode DosipasDocumentProcessor::createNodeFromData(const QByteArray &encodedData) const
{
    ExtractorDocumentNode node;
    if (auto container = Dosipas::v2::UicBarcodeHeader(encodedData); container.isValid()) {
        node.setContent(container);
    } else if (auto container = Dosipas::v1::UicBarcodeHeader(encodedData); container.isValid()) {
        node.setContent(container);
    }
    return node;
}
