/*
    SPDX-FileCopyrightText: 2018-2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "pdfdocumentprocessor.h"

#include <KItinerary/BarcodeDecoder>
#include <KItinerary/ExtractorDocumentNodeFactory>
#include <KItinerary/ExtractorEngine>
#include <KItinerary/PdfDocument>

#include <QImage>
#include <QJSEngine>

#include <unordered_set>

using namespace KItinerary;

Q_DECLARE_METATYPE(KItinerary::Internal::OwnedPtr<KItinerary::PdfDocument>)

enum {
    MaxPageCount = 10, // maximum in the current test set is 6
    MaxFileSize = 4000000, // maximum in the current test set is 980kB
    // unit is 1/72 inch, assuming landscape orientation
    MinTargetImageHeight = 28,
    MinTargetImageWidth = 36,
    MaxTargetImageHeight = 252,
    MaxTargetImageWidth = 252,
};

PdfDocumentProcessor::PdfDocumentProcessor() = default;
PdfDocumentProcessor::~PdfDocumentProcessor() = default;

bool PdfDocumentProcessor::canHandleData(const QByteArray &encodedData, QStringView fileName) const
{
    return PdfDocument::maybePdf(encodedData) || fileName.endsWith(QLatin1String(".pdf"), Qt::CaseInsensitive);
}

static void applyContextDateTime(PdfDocument *pdf, ExtractorDocumentNode &node)
{
    auto dt = pdf->modificationTime();
    if (!dt.isValid()) {
        dt = pdf->creationTime();
    }
    if (dt.isValid() && dt.date().year() > 2000 && dt < QDateTime::currentDateTime()) {
        node.setContextDateTime(dt);
    }
}

ExtractorDocumentNode PdfDocumentProcessor::createNodeFromData(const QByteArray &encodedData) const
{
    auto pdf = PdfDocument::fromData(encodedData);
    // stay away from documents that are atypically large for what we are looking for
    // that's just unnecessarily eating up resources
    if (!pdf || pdf->pageCount() > MaxPageCount || pdf->fileSize() > MaxFileSize) {
        delete pdf;
        return {};
    }

    ExtractorDocumentNode node;
    node.setContent<Internal::OwnedPtr<PdfDocument>>(pdf);
    applyContextDateTime(pdf, node);
    return node;
}

ExtractorDocumentNode PdfDocumentProcessor::createNodeFromContent(const QVariant &decodedData) const
{
    auto pdf = decodedData.value<PdfDocument*>();
    // stay away from documents that are atypically large for what we are looking for
    // that's just unnecessarily eating up resources
    if (!pdf || pdf->pageCount() > MaxPageCount || pdf->fileSize() > MaxFileSize) {
        return {};
    }

    ExtractorDocumentNode node;
    node.setContent(pdf);
    applyContextDateTime(pdf, node);
    return node;
}

static bool maybeBarcode(const PdfImage &img)
{
    const auto w = img.width();
    const auto h = img.height();

    if (!BarcodeDecoder::isPlausibleSize(img.sourceWidth(), img.sourceHeight()) || !BarcodeDecoder::isPlausibleAspectRatio(w, h)) {
        return false;
    }

    // image target size checks
    if (std::min(w, h) < MinTargetImageHeight || std::max(w, h) < MinTargetImageWidth || h > MaxTargetImageHeight || w > MaxTargetImageWidth) {
        return false;
    }

    return true;
}

void PdfDocumentProcessor::expandNode(ExtractorDocumentNode &node, const ExtractorEngine *engine) const
{
    const auto doc = node.content<PdfDocument*>();

    m_imageIds.clear();
    for (int i = 0; i < doc->pageCount(); ++i) {
        const auto page = doc->page(i);

        for (int j = 0; j < page.imageCount(); ++j) {
            auto img = page.image(j);
            img.setLoadingHints(PdfImage::AbortOnColorHint | PdfImage::ConvertToGrayscaleHint); // we only care about b/w-ish images for barcode detection
            if (img.hasObjectId() &&  m_imageIds.find(img.objectId()) != m_imageIds.end()) {
                continue;
            }

            if (!maybeBarcode(img)) {
                continue;
            }

            const auto imgData = img.image();
            if (imgData.isNull()) { // can happen due to AbortOnColorHint
                continue;
            }

            auto childNode = engine->documentNodeFactory()->createNode(imgData, u"internal/qimage");
            childNode.setLocation(i);
            node.appendChild(childNode); // TODO the old code de-duplicated repeated barcodes here - do we actually need that?
            if (img.hasObjectId()) {
                m_imageIds.insert(img.objectId());
            }
        }
    }

    // fallback node for implicit conversion to plain text
    auto fallback = engine->documentNodeFactory()->createNode(doc->text(), u"text/plain");
    node.appendChild(fallback);
}

QJSValue PdfDocumentProcessor::contentToScriptValue(const ExtractorDocumentNode &node, QJSEngine *engine) const
{
    return engine->toScriptValue(node.content<PdfDocument*>());
}

void PdfDocumentProcessor::destroyNode(ExtractorDocumentNode &node) const
{
    destroyIfOwned<PdfDocument>(node);
}
