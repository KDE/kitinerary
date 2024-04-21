/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "pdfimage.h"
#include "pdfimage_p.h"
#include "pdfdocument_p.h"
#include "popplerglobalparams_p.h"
#include "popplerutils_p.h"

#include <QDebug>
#include <QScopedValueRollback>

#include <Gfx.h>
#include <GlobalParams.h>
#include <PDFDoc.h>
#include <Stream.h>
#include <OutputDev.h>

using namespace KItinerary;

static inline bool isColor(GfxRGB rgb)
{
    enum { Threshold = 72 * 256 }; // GfxComp is stored as color value * 255

    // barcode images for SNCF and Renfe for example are anti-aliased, so we cannot simply filter for black or white
    // KLM/AF use tinted barcodes, so checking for R = G = B doesn't help either
    return std::abs(rgb.r - rgb.g) > Threshold || std::abs(rgb.r - rgb.b) > Threshold || std::abs(rgb.g - rgb.b) > Threshold;
}

QImage PdfImagePrivate::load(Stream* str, GfxImageColorMap* colorMap)
{
    if (m_format == QImage::Format_Mono) { // bitmasks are not stored as image streams
        auto img = QImage(m_sourceWidth, m_sourceHeight, QImage::Format_Mono); // TODO implicit Format_Grayscale8 conversion
        str->reset();
        const int rowSize = (m_sourceWidth + 7) / 8;
        for (int y = 0; y < m_sourceHeight; ++y) {
            auto imgData = img.scanLine(y);
            for (int x = 0; x < rowSize; x++) {
                const auto c = str->getChar();
                *imgData++ = c ^ 0xff;
            }
        }

        if (!m_ref.isNull()) {
            m_page->m_doc->m_imageData[m_ref] = img;
        } else {
            m_inlineImageData = img;
        }
        return img;
    }

    auto img = QImage(m_sourceWidth, m_sourceHeight, (m_loadingHints & PdfImage::ConvertToGrayscaleHint) ? QImage::Format_Grayscale8 : m_format);
    const auto bytesPerPixel = colorMap->getNumPixelComps();
    std::unique_ptr<ImageStream> imgStream(new ImageStream(str, m_sourceWidth, bytesPerPixel, colorMap->getBits()));
    imgStream->reset();

    switch (m_format) {
        case QImage::Format_RGB888:
            for (int i = 0; i < m_sourceHeight; ++i) {
                const auto row = imgStream->getLine();
                auto imgData = img.scanLine(i);
                GfxRGB rgb;
                for (int j = 0; j < m_sourceWidth; ++j) {
                    colorMap->getRGB(row + (j * bytesPerPixel), &rgb);
                    if ((m_loadingHints & PdfImage::AbortOnColorHint) && isColor(rgb)) {
                        return {};
                    }
                    if ((m_loadingHints & PdfImage::ConvertToGrayscaleHint)) {
                        *imgData++ = colToByte(rgb.g); // technically not correct but good enough
                    } else {
                        *imgData++ = colToByte(rgb.r);
                        *imgData++ = colToByte(rgb.g);
                        *imgData++ = colToByte(rgb.b);
                    }
                }
            }
            break;
        case QImage::Format_Grayscale8:
            for (int i = 0; i < m_sourceHeight; ++i) {
                const auto row = imgStream->getLine();
                auto imgData = img.scanLine(i);
                GfxGray gray;
                for (int j = 0; j < m_sourceWidth; ++j) {
                    colorMap->getGray(row + j, &gray);
                    *imgData++ = m_ref.m_type == PdfImageType::SMask ? (colToByte(gray) ^ 0xff) : colToByte(gray);
                }
            }
            break;
        default:
            break;
    }
    imgStream->close();

    if (!m_ref.isNull()) {
        m_page->m_doc->m_imageData[m_ref] = img;
    } else {
        m_inlineImageData = img;
    }
    return img;
}

QImage PdfImagePrivate::load()
{
    const auto it = m_page->m_doc->m_imageData.find(m_ref);
    if (it != m_page->m_doc->m_imageData.end()) {
        return (*it).second;
    }

    PopplerGlobalParams gp;

    const auto xref = m_page->m_doc->m_popplerDoc->getXRef();
    const auto obj = xref->fetch(refNum(), refGen());

    switch (m_ref.m_type) {
        case PdfImageType::Image:
            return load(obj.getStream(), m_colorMap.get());
        case PdfImageType::Mask:
        {
            const auto dict = obj.getStream()->getDict();
            const auto maskObj = dict->lookup("Mask");
            return load(maskObj.getStream(), m_colorMap.get());
        }
        case PdfImageType::SMask:
        {
            const auto dict = obj.getStream()->getDict();
            const auto maskObj = dict->lookup("SMask");
            return load(maskObj.getStream(), m_colorMap.get());
        }
    }

    return {};
}


PdfImage::PdfImage()
    : d(new PdfImagePrivate)
{
}

PdfImage::PdfImage(const PdfImage&) = default;
PdfImage::~PdfImage() = default;
PdfImage& PdfImage::operator=(const PdfImage&) = default;

int PdfImage::height() const
{
    if (d->m_format == QImage::Format_Invalid) {
        return d->m_height;
    }
    return d->m_transform.map(QRectF(0, 0, 1, -1)).boundingRect().height();
}

int PdfImage::width() const
{
    if (d->m_format == QImage::Format_Invalid) {
        return d->m_width;
    }
    return d->m_transform.map(QRectF(0, 0, 1, -1)).boundingRect().width();
}

int PdfImage::sourceHeight() const
{
    return d->m_sourceHeight;
}

int PdfImage::sourceWidth() const
{
    return d->m_sourceWidth;
}

QTransform PdfImage::transform() const
{
    return d->m_transform;
}

void PdfImage::setLoadingHints(LoadingHints hints)
{
    d->m_loadingHints = hints;
}

QImage PdfImage::image() const
{
    if (!d->m_inlineImageData.isNull()) {
        return d->m_inlineImageData;
    }
    if (d->m_format == QImage::Format_Invalid) {
        return d->m_vectorPicture.renderToImage();
    }
    return d->load();
}

bool PdfImage::hasObjectId() const
{
    return !d->m_ref.isNull();
}

PdfImageRef PdfImage::objectId() const
{
    return d->m_ref;
}

bool PdfImage::isVectorImage() const
{
    return !d->m_vectorPicture.isNull();
}

int PdfImage::pathElementsCount() const
{
    return d->m_vectorPicture.pathElementsCount();
}

bool PdfImage::hasAspectRatioTransform() const
{
    return d->m_format != QImage::Format_Invalid && (d->m_width != d->m_sourceWidth || d->m_height != d->m_sourceHeight);
}

QImage PdfImage::applyAspectRatioTransform(const QImage &image) const
{
    return image.scaled(d->m_width, d->m_height);
}

PdfImageType PdfImage::type() const
{
    return d->m_ref.m_type;
}

#include "moc_pdfimage.cpp"
