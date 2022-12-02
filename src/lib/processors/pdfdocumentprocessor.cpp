/*
    SPDX-FileCopyrightText: 2018-2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "pdfdocumentprocessor.h"
#include "barcodedocumentprocessorhelper.h"
#include "pdf/pdfbarcodeutil_p.h"
#include "text/nameoptimizer_p.h"

#include <KItinerary/BarcodeDecoder>
#include <KItinerary/ExtractorDocumentNodeFactory>
#include <KItinerary/ExtractorEngine>
#include <KItinerary/ExtractorResult>
#include <KItinerary/PdfDocument>

#include <QImage>
#include <QJSEngine>


using namespace KItinerary;

Q_DECLARE_METATYPE(KItinerary::Internal::OwnedPtr<KItinerary::PdfDocument>)

enum {
    MaxPageCount = 10, // maximum in the current test set is 6
    MaxFileSize = 4000000, // maximum in the current test set is 980kB
};

PdfDocumentProcessor::PdfDocumentProcessor() = default;
PdfDocumentProcessor::~PdfDocumentProcessor() = default;

bool PdfDocumentProcessor::canHandleData(const QByteArray &encodedData, QStringView fileName) const
{
    return PdfDocument::maybePdf(encodedData) || fileName.endsWith(QLatin1String(".pdf"), Qt::CaseInsensitive);
}

static void applyContextDateTime(PdfDocument *pdf, ExtractorDocumentNode &node)
{
    // ignore broken PDF times for Amadeus documents
    if (pdf->producer() == QLatin1String("Amadeus") && pdf->creationTime() == pdf->modificationTime() && pdf->creationTime().date() == QDate(2011, 5, 10)) {
        return;
    }

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

void PdfDocumentProcessor::expandNode(ExtractorDocumentNode &node, const ExtractorEngine *engine) const
{
    const auto doc = node.content<PdfDocument*>();

    for (int i = 0; i < doc->pageCount(); ++i) {
        const auto page = doc->page(i);
        m_imageIds.clear();

        for (int j = 0; j < page.imageCount(); ++j) {
            auto img = page.image(j);
            img.setLoadingHints(PdfImage::AbortOnColorHint | PdfImage::ConvertToGrayscaleHint); // we only care about b/w-ish images for barcode detection
            if (img.hasObjectId() &&  m_imageIds.find(img.objectId()) != m_imageIds.end()) {
                continue;
            }

            const auto barcodeHints = PdfBarcodeUtil::maybeBarcode(img, BarcodeDecoder::Any2D | BarcodeDecoder::Any1D);
            if (barcodeHints == BarcodeDecoder::None) {
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

            // technically not our job to do this here rather than letting the image node processor handle this
            // but we have the output aspect ratio of the barcode only here, which gives better decoding hints
            BarcodeDocumentProcessorHelper::expandNode(imgData, barcodeHints, childNode, engine);
        }

        // handle full page raster images
        if ((engine->hints() & ExtractorEngine::ExtractFullPageRasterImages) && page.imageCount() == 1 && page.text().isEmpty()) {
            qDebug() << "full page raster image";
            auto img = page.image(0);
            if (img.hasObjectId() &&  m_imageIds.find(img.objectId()) != m_imageIds.end()) { // already handled
                continue;
            }

            img.setLoadingHints(PdfImage::NoHint); // don't abort on color
            const auto imgData = img.image();
            if (imgData.isNull()) {
                continue;
            }

            auto childNode = engine->documentNodeFactory()->createNode(imgData, u"internal/qimage");
            childNode.setLocation(i);
            node.appendChild(childNode);
            if (img.hasObjectId()) {
                m_imageIds.insert(img.objectId());
            }
        }
    }

    // fallback node for implicit conversion to plain text
    auto fallback = engine->documentNodeFactory()->createNode(doc->text(), u"text/plain");
    node.appendChild(fallback);
}

void PdfDocumentProcessor::postExtract(ExtractorDocumentNode &node) const
{
    // find the text node we can run the optimizer on
    if (node.childNodes().empty()) {
        return;
    }
    const QString text = node.childNodes().back().content<QString>();

    // run name optimizer on all results
    QVector<QVariant> result;
    const auto res = node.result().result();
    result.reserve(res.size());
    for (const auto &r : res) {
        result.push_back(NameOptimizer::optimizeNameRecursive(text, r));
    }
    node.setResult(std::move(result));
}

QJSValue PdfDocumentProcessor::contentToScriptValue(const ExtractorDocumentNode &node, QJSEngine *engine) const
{
    return engine->toScriptValue(node.content<PdfDocument*>());
}

void PdfDocumentProcessor::destroyNode(ExtractorDocumentNode &node) const
{
    destroyIfOwned<PdfDocument>(node);
}
