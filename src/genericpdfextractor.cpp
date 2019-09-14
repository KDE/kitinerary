/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "genericpdfextractor_p.h"
#include "genericuic918extractor_p.h"

#include <KItinerary/BarcodeDecoder>
#include <KItinerary/IataBcbpParser>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/PdfDocument>
#include <KItinerary/Uic9183Parser>

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

void GenericPdfExtractor::extract(PdfDocument *doc, QJsonArray &result)
{
    m_unrecognizedBarcodes.clear();

    // stay away from documents that are atypically large for what we are looking for
    // that's just unnecessarily eating up resources
    if (doc->pageCount() > MaxPageCount || doc->fileSize() > MaxFileSize) {
        return;
    }

    m_imageIds.clear();
    for (int i = 0; i < doc->pageCount(); ++i) {
        const auto page = doc->page(i);

        for (int j = 0; j < page.imageCount(); ++j) {
            const auto img = page.image(j);
            if (img.hasObjectId() &&  m_imageIds.find(img.objectId()) != m_imageIds.end()) {
                continue;
            }

            if (!maybeBarcode(img)) {
                continue;
            }

            extractImage(img, result);
            if (img.hasObjectId()) {
                m_imageIds.insert(img.objectId());
            }
        }
    }
}

QStringList GenericPdfExtractor::unrecognizedBarcodes() const
{
    return m_unrecognizedBarcodes;
}

void GenericPdfExtractor::extractImage(const PdfImage &img, QJsonArray &result)
{
    const auto b = m_barcodeDecoder->decodeBinary(img.image());
    if (Uic9183Parser::maybeUic9183(b)) {
        GenericUic918Extractor::extract(b, result, m_contextDate);
        return;
    }

    if (b.isEmpty()) {
        extractBarcode(m_barcodeDecoder->decodeString(img.image()), result);
    } else {
        extractBarcode(QString::fromUtf8(b), result);
    }
}

void GenericPdfExtractor::extractBarcode(const QString &code, QJsonArray &result)
{
    if (code.isEmpty()) {
        return;
    }

    if (IataBcbpParser::maybeIataBcbp(code)) {
        const auto res = IataBcbpParser::parse(code, m_contextDate.date());
        const auto jsonLd = JsonLdDocument::toJson(res);
        std::copy(jsonLd.begin(), jsonLd.end(), std::back_inserter(result));
    }

    m_unrecognizedBarcodes.push_back(code);
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
