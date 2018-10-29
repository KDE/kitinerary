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
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "config-kitinerary.h"
#include "pdfdocument.h"

#include <QDebug>
#include <QImage>
#include <QScopedValueRollback>

#ifdef HAVE_POPPLER
#include <GlobalParams.h>
#include <PDFDoc.h>
#include <Stream.h>
#include <TextOutputDev.h>
#endif

#include <cmath>
#include <unordered_map>

using namespace KItinerary;

namespace KItinerary {
class PdfImagePrivate : public QSharedData {
public:
#ifdef HAVE_POPPLER
    QImage load(Stream *str, GfxImageColorMap *colorMap);

    std::unique_ptr<GfxImageColorMap> m_colorMap;
#endif

    int m_refNum = -1;
    int m_refGen = -1;
    PdfPagePrivate *m_page = nullptr;
    QTransform m_transform;
    int m_width = 0;
    int m_height = 0;
    int m_sourceWidth = 0;
    int m_sourceHeight = 0;
    QImage::Format m_format = QImage::Format_Invalid;
};

class PdfPagePrivate : public QSharedData {
public:
    void load();

    int m_pageNum = -1;
    bool m_loaded = false;
    QString m_text;
    std::vector<PdfImage> m_images;
    PdfDocumentPrivate *m_doc;
};

class PdfDocumentPrivate {
public:
    // needs to be kept alive as long as the Poppler::PdfDoc instance lives
    QByteArray m_pdfData;
    // this contains the actually loaded/decoded image data
    // and is referenced by the object id from PdfImage to avoid
    // expensive loading/decoding of multiple occurrences of the same image
    // image data in here is stored in its source form, without applied transformations
    std::unordered_map<int, QImage> m_imageData;
    std::vector<PdfPage> m_pages;
#ifdef HAVE_POPPLER
    std::unique_ptr<PDFDoc> m_popplerDoc;
#endif
};

#ifdef HAVE_POPPLER
static std::unique_ptr<GlobalParams> s_globalParams;
static GlobalParams* popplerGlobalParams()
{
    if (!s_globalParams) {
        s_globalParams.reset(new GlobalParams);
    }
    return s_globalParams.get();
}

class ExtractorOutputDevice : public TextOutputDev
{
public:
    ExtractorOutputDevice();
    bool needNonText() override { return true; }
    void drawImage(GfxState *state, Object *ref, Stream *str, int width, int height, GfxImageColorMap *colorMap, bool interpolate, int *maskColors, bool inlineImg) override;

    std::vector<PdfImage> m_images;

private:
    QTransform m_transform;
};

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
#endif
#endif

}

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


ExtractorOutputDevice::ExtractorOutputDevice()
    : TextOutputDev(nullptr, false, 0, false, false)
{
}

void ExtractorOutputDevice::drawImage(GfxState* state, Object* ref, Stream* str, int width, int height, GfxImageColorMap* colorMap, bool interpolate, int* maskColors, bool inlineImg)
{
    Q_UNUSED(str);
    Q_UNUSED(interpolate);
    Q_UNUSED(maskColors);
    Q_UNUSED(inlineImg);

    if (!colorMap || !colorMap->isOk() || !ref || !ref->isRef()) {
        return;
    }

    QImage::Format format;
    if (colorMap->getColorSpace()->getMode() == csIndexed) {
        format = QImage::Format_RGB888;
    } else if (colorMap->getNumPixelComps() == 1 && (colorMap->getBits() >= 1 && colorMap->getBits() <= 8)) {
        format = QImage::Format_Grayscale8;
    } else if (colorMap->getNumPixelComps() == 3 && colorMap->getBits() == 8) {
        format = QImage::Format_RGB888;
    } else {
        return;
    }

    PdfImage pdfImg;
    pdfImg.d->m_refNum = ref->getRef().num;
    pdfImg.d->m_refGen = ref->getRef().gen;

#ifdef HAVE_POPPLER_0_69
    pdfImg.d->m_colorMap.reset(colorMap->copy());
#endif
    pdfImg.d->m_sourceHeight = height;
    pdfImg.d->m_sourceWidth = width;
    pdfImg.d->m_width = width;
    pdfImg.d->m_height = height;
    // deal with aspect-ratio changing scaling
    const auto sourceAspectRatio = (double)width / (double)height;
    const auto targetAspectRatio = state->getCTM()[0] / -state->getCTM()[3];
    if (!qFuzzyCompare(sourceAspectRatio, targetAspectRatio) && qFuzzyIsNull(state->getCTM()[1]) && qFuzzyIsNull(state->getCTM()[2])) {
        if (targetAspectRatio > sourceAspectRatio) {
            pdfImg.d->m_width = width * targetAspectRatio / sourceAspectRatio;
        } else {
            pdfImg.d->m_height = height * sourceAspectRatio / targetAspectRatio;
        }
    }
    const auto ctm = state->getCTM();
    pdfImg.d->m_transform = QTransform(ctm[0], ctm[1], ctm[2], ctm[3], ctm[4], ctm[5]);
    pdfImg.d->m_format = format;
    m_images.push_back(pdfImg);
}

#ifndef HAVE_POPPLER_0_69
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
    QScopedValueRollback<GlobalParams*> globalParamResetter(globalParams, popplerGlobalParams());

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

int PdfImage::objectId() const
{
    return d->m_refNum;
}


void PdfPagePrivate::load()
{
    if (m_loaded) {
        return;
    }

#ifdef HAVE_POPPLER
    QScopedValueRollback<GlobalParams*> globalParamResetter(globalParams, popplerGlobalParams());
    ExtractorOutputDevice device;
    m_doc->m_popplerDoc->displayPageSlice(&device, m_pageNum + 1, 72, 72, 0, false, true, false, -1, -1, -1, -1);
    const auto pageRect = m_doc->m_popplerDoc->getPage(m_pageNum + 1)->getCropBox();
    std::unique_ptr<GooString> s(device.getText(pageRect->x1, pageRect->y1, pageRect->x2, pageRect->y2));

    m_text = QString::fromUtf8(s->getCString());
    m_images = std::move(device.m_images);
    for (auto it = m_images.begin(); it != m_images.end(); ++it) {
        (*it).d->m_page = this;
    }
#endif
    m_loaded = true;
}

PdfPage::PdfPage()
    : d(new PdfPagePrivate)
{
}

PdfPage::PdfPage(const PdfPage&) = default;
PdfPage::~PdfPage() = default;
PdfPage& PdfPage::operator=(const PdfPage&) = default;

QString PdfPage::text() const
{
    d->load();
    return d->m_text;
}

#ifdef HAVE_POPPLER
static double ratio(double begin, double end, double ratio)
{
    return begin + (end - begin) * ratio;
}
#endif

QString PdfPage::textInRect(double left, double top, double right, double bottom) const
{
#ifdef HAVE_POPPLER
    QScopedValueRollback<GlobalParams*> globalParamResetter(globalParams, popplerGlobalParams());

    ExtractorOutputDevice device;
    d->m_doc->m_popplerDoc->displayPageSlice(&device, d->m_pageNum + 1, 72, 72, 0, false, true, false, -1, -1, -1, -1);
    const auto pageRect = d->m_doc->m_popplerDoc->getPage(d->m_pageNum + 1)->getCropBox();
    std::unique_ptr<GooString> s(device.getText(ratio(pageRect->x1, pageRect->x2, left), ratio(pageRect->y1, pageRect->y2, top),
                                                ratio(pageRect->x1, pageRect->x2, right), ratio(pageRect->y1, pageRect->y2, bottom)));
    return QString::fromUtf8(s->getCString());
#else
    Q_UNUSED(left);
    Q_UNUSED(top);
    Q_UNUSED(right);
    Q_UNUSED(bottom);
    return {};
#endif
}

int PdfPage::imageCount() const
{
    d->load();
    return d->m_images.size();
}

PdfImage PdfPage::image(int index) const
{
    d->load();
    return d->m_images[index];
}

QVariantList PdfPage::imagesVariant() const
{
    d->load();
    QVariantList l;
    l.reserve(imageCount());
    std::for_each(d->m_images.begin(), d->m_images.end(), [&l](const PdfImage& img) { l.push_back(QVariant::fromValue(img)); });
    return l;
}

QVariantList PdfPage::imagesInRect(double left, double top, double right, double bottom) const
{
    d->load();
    QVariantList l;
#ifdef HAVE_POPPLER
    QScopedValueRollback<GlobalParams*> globalParamResetter(globalParams, popplerGlobalParams());
    const auto pageRect = d->m_doc->m_popplerDoc->getPage(d->m_pageNum + 1)->getCropBox();

    for (const auto &img : d->m_images) {
        if ((img.d->m_transform.dx() >= ratio(pageRect->x1, pageRect->x2, left) && img.d->m_transform.dx() <= ratio(pageRect->x1, pageRect->x2, right)) &&
            (img.d->m_transform.dy() >= ratio(pageRect->y1, pageRect->y2, top)  && img.d->m_transform.dy() <= ratio(pageRect->y1, pageRect->y2, bottom)))
        {
            l.push_back(QVariant::fromValue(img));
        }
    }
#else
    Q_UNUSED(left);
    Q_UNUSED(top);
    Q_UNUSED(right);
    Q_UNUSED(bottom);
#endif
    return l;
}


PdfDocument::PdfDocument(QObject *parent)
    : QObject(parent)
    , d(new PdfDocumentPrivate)
{
}

PdfDocument::~PdfDocument() = default;

QString PdfDocument::text() const
{
    QString text;
    std::for_each(d->m_pages.begin(), d->m_pages.end(), [&text](const PdfPage &p) { text += p.text(); });
    return text;
}

int PdfDocument::pageCount() const
{
#ifdef HAVE_POPPLER
    return d->m_popplerDoc->getNumPages();
#else
    return 0;
#endif
}

PdfPage PdfDocument::page(int index) const
{
    return d->m_pages[index];
}

int PdfDocument::fileSize() const
{
    return d->m_pdfData.size();
}

QVariantList PdfDocument::pagesVariant() const
{
    QVariantList l;
    l.reserve(pageCount());
    std::for_each(d->m_pages.begin(), d->m_pages.end(), [&l](const PdfPage& p) { l.push_back(QVariant::fromValue(p)); });
    return l;
}

PdfDocument* PdfDocument::fromData(const QByteArray &data, QObject *parent)
{
#ifdef HAVE_POPPLER
    QScopedValueRollback<GlobalParams*> globalParamResetter(globalParams, popplerGlobalParams());

    std::unique_ptr<PdfDocument> doc(new PdfDocument(parent));
    doc->d->m_pdfData = data;
    // PDFDoc takes ownership of stream
#ifdef HAVE_POPPLER_0_58
    auto stream = new MemStream(const_cast<char*>(doc->d->m_pdfData.constData()), 0, doc->d->m_pdfData.size(), Object());
#else
    Object obj;
    obj.initNull();
    auto stream = new MemStream(const_cast<char*>(doc->d->m_pdfData.constData()), 0, doc->d->m_pdfData.size(), &obj);
#endif
    std::unique_ptr<PDFDoc> popplerDoc(new PDFDoc(stream, nullptr, nullptr));
    if (!popplerDoc->isOk()) {
        return nullptr;
    }

    doc->d->m_pages.reserve(popplerDoc->getNumPages());
    for (int i = 0; i < popplerDoc->getNumPages(); ++i) {
        PdfPage page;
        page.d->m_pageNum = i;
        page.d->m_doc = doc->d.get();
        doc->d->m_pages.push_back(page);
    }

    doc->d->m_popplerDoc = std::move(popplerDoc);
    return doc.release();
#else
    Q_UNUSED(data);
    Q_UNUSED(parent);
    return nullptr;
#endif
}
