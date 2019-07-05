/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

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

#include "pdfimage.h"
#include "pdfimage_p.h"
#include "pdfdocument_p.h"
#include "popplerutils_p.h"

#include <QScopedValueRollback>

#ifdef HAVE_POPPLER
#include <Gfx.h>
#include <GlobalParams.h>
#include <PDFDoc.h>
#include <Stream.h>
#endif

using namespace KItinerary;

#ifdef HAVE_POPPLER
QImage PdfImagePrivate::load(Stream* str, GfxImageColorMap* colorMap)
{
    auto img = QImage(m_sourceWidth, m_sourceHeight, m_format);
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
                    *imgData++ = colToByte(rgb.r);
                    *imgData++ = colToByte(rgb.g);
                    *imgData++ = colToByte(rgb.b);
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

// legacy image loading
#ifndef HAVE_POPPLER_0_69
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
    Q_UNUSED(state);
    Q_UNUSED(height);
    Q_UNUSED(width);
    Q_UNUSED(interpolate);
    Q_UNUSED(maskColors);
    Q_UNUSED(inlineImg);

    if (!colorMap || !colorMap->isOk() || !ref) {
        return;
    }

    if (ref->isRef() && d->m_refNum != ref->getRef().num) {
        return;
    }

    m_image = d->load(str, colorMap);
}
#endif
#endif

PdfImage::PdfImage()
    : d(new PdfImagePrivate)
{
}

PdfImage::PdfImage(const PdfImage&) = default;
PdfImage::~PdfImage() = default;
PdfImage& PdfImage::operator=(const PdfImage&) = default;

int PdfImage::height() const
{
    return d->m_height;
}

int PdfImage::width() const
{
    return d->m_width;
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

QImage PdfImage::sourceImage() const
{
    const auto it = d->m_page->m_doc->m_imageData.find(d->m_refNum);
    if (it != d->m_page->m_doc->m_imageData.end()) {
        return (*it).second;
    }

#ifdef HAVE_POPPLER
    QScopedValueRollback<GlobalParams*> globalParamResetter(globalParams, PopplerUtils::globalParams());

#ifdef HAVE_POPPLER_0_69
    const auto xref = d->m_page->m_doc->m_popplerDoc->getXRef();
    const auto obj = xref->fetch(d->m_refNum, d->m_refGen);
    return d->load(obj.getStream(), d->m_colorMap.get());
#else
    std::unique_ptr<ImageLoaderOutputDevice> device(new ImageLoaderOutputDevice(d.data()));
    d->m_page->m_doc->m_popplerDoc->displayPageSlice(device.get(), d->m_page->m_pageNum + 1, 72, 72, 0, false, true, false, -1, -1, -1, -1);
    return device->image();
#endif

#else
    return {};
#endif
}

QImage PdfImage::image() const
{
    const auto img = sourceImage();
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
