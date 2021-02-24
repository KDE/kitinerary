/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "genericpdfextractor_p.h"
#include "genericuic918extractor_p.h"
#include "genericvdvextractor_p.h"

#include <KItinerary/BarcodeDecoder>
#include <KItinerary/IataBcbpParser>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/PdfDocument>
#include <KItinerary/Uic9183Parser>
#include <KItinerary/VdvTicketParser>

#include <QDebug>
#include <QImage>
#include <QJsonArray>
#include <QJsonObject>
#include <QTransform>

using namespace KItinerary;

enum {
    MaxPageCount = 10, // maximum in the current test set is 6
    MaxFileSize = 4000000, // maximum in the current test set is 980kB
    // unit is 1/72 inch, assuming landscape orientation
    MinTargetImageHeight = 28,
    MinTargetImageWidth = 36,
    MaxTargetImageHeight = 252,
    MaxTargetImageWidth = 252,
};

GenericPdfExtractor::GenericPdfExtractor() = default;
GenericPdfExtractor::~GenericPdfExtractor() = default;

void GenericPdfExtractor::setBarcodeDecoder(BarcodeDecoder *decoder)
{
    m_barcodeDecoder = decoder;
}

void GenericPdfExtractor::setContextDate(const QDateTime &dt)
{
    m_contextDate = dt;
}

std::vector<GenericExtractor::Result> GenericPdfExtractor::extract(PdfDocument *doc)
{
    std::vector<GenericExtractor::Result> result;

    // stay away from documents that are atypically large for what we are looking for
    // that's just unnecessarily eating up resources
    if (doc->pageCount() > MaxPageCount || doc->fileSize() > MaxFileSize) {
        return result;
    }

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

            auto r = extractImage(img, result);
            if (!r.isEmpty()) {
                r.setPageNumber(i);
                result.push_back(r);
            }
            if (img.hasObjectId()) {
                m_imageIds.insert(img.objectId());
            }
        }
    }

    return result;
}

static bool containsBarcodeResult(const std::vector<GenericExtractor::Result> &results, const QVariant &barcode)
{
    const auto it = std::find_if(results.begin(), results.end(), [barcode](const auto &result) {
        return result.barcode() == barcode;
    });
    return it != results.end();
}

GenericExtractor::Result GenericPdfExtractor::extractImage(const PdfImage &img, const std::vector<GenericExtractor::Result> &existingResults)
{
    const auto imgData = img.image();
    if (imgData.isNull()) { // can happen due to AbortOnColorHint
        return {};
    }

    // binary barcode content
    const auto b = m_barcodeDecoder->decodeBinary(imgData);
    if (!b.isEmpty()) {
        if (containsBarcodeResult(existingResults, b)) {
            return {};
        }

        if (Uic9183Parser::maybeUic9183(b)) {
            QJsonArray result;
            GenericUic918Extractor::extract(b, result);
            if (!result.isEmpty()) {
                return GenericExtractor::Result{result, b};
            }
            return {};
        }

        if (VdvTicketParser::maybeVdvTicket(b)) {
            const auto result = GenericVdvExtractor::extract(b);
            if (!result.isEmpty()) {
                return GenericExtractor::Result{result, b};
            }
        }
    }

    // string barcode content
    const auto s = m_barcodeDecoder->decodeString(imgData);
    if (!s.isEmpty()) {
        if (containsBarcodeResult(existingResults, s)) {
            return {};
        }

        if (IataBcbpParser::maybeIataBcbp(s)) {
            const auto res = IataBcbpParser::parse(s, m_contextDate.date());
            const auto jsonLd = JsonLdDocument::toJson(res);
            return GenericExtractor::Result{jsonLd, s};
        }
    }

    return GenericExtractor::Result{QJsonArray(), s.isEmpty() ? b.isEmpty() ? QVariant() : QVariant(b) : QVariant(s)};
}

bool GenericPdfExtractor::maybeBarcode(const PdfImage &img, BarcodeDecoder::BarcodeTypes hint)
{
    const auto w = img.width();
    const auto h = img.height();

    if (!BarcodeDecoder::isPlausibleSize(img.sourceWidth(), img.sourceHeight()) || !BarcodeDecoder::isPlausibleAspectRatio(w, h, hint)) {
        return false;
    }

    // image target size checks
    if (std::min(w, h) < MinTargetImageHeight || std::max(w, h) < MinTargetImageWidth || h > MaxTargetImageHeight || w > MaxTargetImageWidth) {
        return false;
    }

    return true;
}
