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

#ifdef HAVE_POPPLER
#include <Gfx.h>
#include <GlobalParams.h>
#include <PDFDoc.h>
#include <Stream.h>
#include <OutputDev.h>
#endif

using namespace KItinerary;

#ifdef HAVE_POPPLER
// legacy image loading
#if KPOPPLER_VERSION < QT_VERSION_CHECK(0, 69, 0)
namespace KItinerary {
class ImageLoaderOutputDevice : public OutputDev
{
public:
    ImageLoaderOutputDevice(PdfImagePrivate *dd);

    bool interpretType3Chars() override { return false; }
    bool needNonText() override { return true; }
    bool upsideDown() override { return false; }
    bool useDrawChar() override { return false; }

    void drawImage(GfxState *state, Object *ref, Stream *str, int width, int height, GfxImageColorMap *colorMap, bool interpolate, int *maskColors, bool inlineImg) override;
    QImage image() const { return m_image; }

private:
    PdfImagePrivate *d;
    QImage m_image;
};

ImageLoaderOutputDevice::ImageLoaderOutputDevice(PdfImagePrivate* dd)
    : d(dd)
{
}

void ImageLoaderOutputDevice::drawImage(GfxState *state, Object *ref, Stream *str, int width, int height, GfxImageColorMap *colorMap, bool interpolate, int *maskColors, bool inlineImg)
{
    Q_UNUSED(state)
    Q_UNUSED(height)
    Q_UNUSED(width)
    Q_UNUSED(interpolate)
    Q_UNUSED(maskColors)
    Q_UNUSED(inlineImg)

    if (!colorMap || !colorMap->isOk() || !ref) {
        return;
    }

    if (ref->isRef() && d->m_refNum != ref->getRef().num) {
        return;
    }

    m_image = d->load(str, colorMap);
}
}
#endif

static inline bool isColor(GfxRGB rgb)
{
    enum { Threshold = 72 * 256 }; // GfxComp is stored as color value * 255

    // barcode images for SNCF and Renfe for example are anti-aliased, so we cannot simply filter for black or white
    // KLM/AF use tinted barcodes, so checking for R = G = B doesn't help either
    return std::abs(rgb.r - rgb.g) > Threshold || std::abs(rgb.r - rgb.b) > Threshold || std::abs(rgb.g - rgb.b) > Threshold;
}

QImage PdfImagePrivate::load(Stream* str, GfxImageColorMap* colorMap)
{
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
                    *imgData++ = colToByte(gray);
                }
            }
            break;
        default:
            break;
    }
    imgStream->close();

    m_page->m_doc->m_imageData[m_refNum] = img;
    return img;
}
#endif

QImage PdfImagePrivate::load()
{
    const auto it = m_page->m_doc->m_imageData.find(m_refNum);
    if (it != m_page->m_doc->m_imageData.end()) {
        return (*it).second;
    }

#ifdef HAVE_POPPLER
    PopplerGlobalParams gp;

#if KPOPPLER_VERSION >= QT_VERSION_CHECK(0, 69, 0)
    const auto xref = m_page->m_doc->m_popplerDoc->getXRef();
    const auto obj = xref->fetch(m_refNum, m_refGen);
    return load(obj.getStream(), m_colorMap.get());
#else
    std::unique_ptr<ImageLoaderOutputDevice> device(new ImageLoaderOutputDevice(this));
    m_page->m_doc->m_popplerDoc->displayPageSlice(device.get(), m_page->m_pageNum + 1, 72, 72, 0, false, true, false, -1, -1, -1, -1);
    return device->image();
#endif

#else
    return {};
#endif
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
    if (d->m_format == QImage::Format_Invalid) {
        return d->m_vectorPicture.renderToImage();
    }

    const auto img = d->load();
    if (d->m_width != d->m_sourceWidth || d->m_height != d->m_sourceHeight) {
        return img.scaled(d->m_width, d->m_height);
    }
    return img;
}

bool PdfImage::hasObjectId() const
{
    return d->m_refNum >= 0;
}

int PdfImage::objectId() const
{
    return d->m_refNum;
}