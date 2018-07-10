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

using namespace KItinerary;

namespace KItinerary {
class PdfImagePrivate : public QSharedData {
public:
#ifdef HAVE_POPPLER
    void load(Stream *str, int width, int height, GfxImageColorMap *colorMap);
#endif

    int m_ref = -1;
    PdfPagePrivate *m_page = nullptr;
    QImage m_img;
    double m_x = NAN;
    double m_y = NAN;
    int m_width = 0;
    int m_height = 0;
    QImage::Format m_format = QImage::Format_Invalid;
};

class PdfPagePrivate : public QSharedData {
public:
    int m_pageNum = -1;
    QString m_text;
    std::vector<PdfImage> m_images;
    PdfDocumentPrivate *m_doc;
};

class PdfDocumentPrivate {
public:
    QByteArray m_pdfData; // needs to be kept alive as long as the Poppler::PdfDoc instance lives
    std::vector<PdfImage> m_images;
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
    ExtractorOutputDevice(PdfDocumentPrivate *dd);
    GBool needNonText() override { return true; }
    void drawImage(GfxState *state, Object *ref, Stream *str, int width, int height, GfxImageColorMap *colorMap, GBool interpolate, int *maskColors, GBool inlineImg) override;

    std::vector<PdfImage> m_images;

private:
    PdfDocumentPrivate *d;
};

class ImageLoaderOutputDevice : public OutputDev
{
public:
    ImageLoaderOutputDevice(PdfImagePrivate *dd);

    GBool interpretType3Chars() override { return false; }
    GBool needNonText() override { return true; }
    GBool upsideDown() override { return false; }
    GBool useDrawChar() override { return false; }

    void drawImage(GfxState *state, Object *ref, Stream *str, int width, int height, GfxImageColorMap *colorMap, GBool interpolate, int *maskColors, GBool inlineImg) override;

private:
    PdfImagePrivate *d;
};
#endif

}

#ifdef HAVE_POPPLER
void PdfImagePrivate::load(Stream* str, int width, int height, GfxImageColorMap* colorMap)
{
    m_img = QImage(width, height, m_format);
    const auto bytesPerPixel = colorMap->getNumPixelComps();
    std::unique_ptr<ImageStream> imgStream(new ImageStream(str, width, bytesPerPixel, colorMap->getBits()));
    imgStream->reset();

    switch (m_format) {
        case QImage::Format_RGB888:
            for (int i = 0; i < height; ++i) {
                const auto row = imgStream->getLine();
                auto imgData = m_img.scanLine(i);
                GfxRGB rgb;
                for (int j = 0; j < width; ++j) {
                    colorMap->getRGB(row + (j * bytesPerPixel), &rgb);
                    *imgData++ = colToByte(rgb.r);
                    *imgData++ = colToByte(rgb.g);
                    *imgData++ = colToByte(rgb.b);
                }
            }
            break;
        case QImage::Format_Grayscale8:
            for (int i = 0; i < height; ++i) {
                const auto row = imgStream->getLine();
                auto imgData = m_img.scanLine(i);
                GfxGray gray;
                for (int j = 0; j < width; ++j) {
                    colorMap->getGray(row + j, &gray);
                    *imgData++ = colToByte(gray);
                }
            }
            break;
        default:
            break;
    }
    imgStream->close();

    if (m_width != width || m_height != height) {
        m_img = m_img.scaled(m_width, m_height);
    }
}


ExtractorOutputDevice::ExtractorOutputDevice(PdfDocumentPrivate *dd)
    : TextOutputDev(nullptr, false, 0, false, false)
    , d(dd)
{
}

void ExtractorOutputDevice::drawImage(GfxState* state, Object* ref, Stream* str, int width, int height, GfxImageColorMap* colorMap, GBool interpolate, int* maskColors, GBool inlineImg)
{
    Q_UNUSED(str);
    Q_UNUSED(interpolate);
    Q_UNUSED(maskColors);
    Q_UNUSED(inlineImg);

    if (!colorMap || !colorMap->isOk()) {
        return;
    }

    // check for duplicate occurances of ref->getRef().num
    if (ref->isRef()) {
        auto it = std::find_if(d->m_images.begin(), d->m_images.end(), [ref](const PdfImage &other) {
            return other.d->m_ref == ref->getRef().num;
        });
        if (it != d->m_images.end()) {
            m_images.push_back(*it);
            return;
        }
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
    if (ref->isRef()) {
        pdfImg.d->m_ref = ref->getRef().num;
    }

    pdfImg.d->m_width = width;
    pdfImg.d->m_height = height;
    // deal with aspect-ratio changing scaling
    const auto sourceAspectRatio = (double)width / (double)height;
    const auto targetAspectRatio = state->getCTM()[0] / -state->getCTM()[3];
    if (!qFuzzyCompare(sourceAspectRatio, targetAspectRatio) && qFuzzyIsNull(state->getCTM()[1]) && qFuzzyIsNull(state->getCTM()[2])) {
        if (targetAspectRatio > sourceAspectRatio) {
            pdfImg.d->m_width = height * state->getCTM()[0] / -state->getCTM()[3];
        } else {
            pdfImg.d->m_height = width * -state->getCTM()[3] / state->getCTM()[0];
        }
    }
    pdfImg.d->m_x = state->getCTM()[4];
    pdfImg.d->m_y = state->getCTM()[5];
    pdfImg.d->m_format = format;
    m_images.push_back(pdfImg);
}

ImageLoaderOutputDevice::ImageLoaderOutputDevice(PdfImagePrivate* dd)
    : d(dd)
{
}

void ImageLoaderOutputDevice::drawImage(GfxState *state, Object *ref, Stream *str, int width, int height, GfxImageColorMap *colorMap, GBool interpolate, int *maskColors, GBool inlineImg)
{
    Q_UNUSED(state);
    Q_UNUSED(interpolate);
    Q_UNUSED(maskColors);
    Q_UNUSED(inlineImg);

    if (!colorMap || !colorMap->isOk()) {
        return;
    }

    if (ref->isRef() && d->m_ref != ref->getRef().num) {
        return;
    }

    d->load(str, width, height, colorMap);
}
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

QImage PdfImage::image() const
{
    if (!d->m_img.isNull() || d->m_ref < 0) {
        return d->m_img;
    }

#ifdef HAVE_POPPLER
    QScopedValueRollback<GlobalParams*> globalParamResetter(globalParams, popplerGlobalParams());

    std::unique_ptr<ImageLoaderOutputDevice> device(new ImageLoaderOutputDevice(d.data()));
    d->m_page->m_doc->m_popplerDoc->displayPageSlice(device.get(), d->m_page->m_pageNum + 1, 72, 72, 0, false, true, false, -1, -1, -1, -1);
#endif
    return d->m_img;
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

    std::unique_ptr<ExtractorOutputDevice> device(new ExtractorOutputDevice(d->m_doc));
    d->m_doc->m_popplerDoc->displayPageSlice(device.get(), d->m_pageNum + 1, 72, 72, 0, false, true, false, -1, -1, -1, -1);
    const auto pageRect = d->m_doc->m_popplerDoc->getPage(d->m_pageNum + 1)->getCropBox();
    std::unique_ptr<GooString> s(device->getText(ratio(pageRect->x1, pageRect->x2, left), ratio(pageRect->y1, pageRect->y2, top),
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
    return d->m_images.size();
}

PdfImage PdfPage::image(int index) const
{
    return d->m_images[index];
}

QVariantList PdfPage::imagesVariant() const
{
    QVariantList l;
    l.reserve(imageCount());
    std::for_each(d->m_images.begin(), d->m_images.end(), [&l](const PdfImage& img) { l.push_back(QVariant::fromValue(img)); });
    return l;
}

QVariantList PdfPage::imagesInRect(double left, double top, double right, double bottom) const
{
    QVariantList l;
#ifdef HAVE_POPPLER
    QScopedValueRollback<GlobalParams*> globalParamResetter(globalParams, popplerGlobalParams());
    const auto pageRect = d->m_doc->m_popplerDoc->getPage(d->m_pageNum + 1)->getCropBox();

    for (const auto &img : d->m_images) {
        if ((img.d->m_x >= ratio(pageRect->x1, pageRect->x2, left) && img.d->m_x <= ratio(pageRect->x1, pageRect->x2, right)) &&
            (img.d->m_y >= ratio(pageRect->y1, pageRect->y2, top)  && img.d->m_y <= ratio(pageRect->y1, pageRect->y2, bottom)))
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

int PdfDocument::imageCount() const
{
    return d->m_images.size();
}

PdfImage PdfDocument::image(int index) const
{
    return d->m_images[index];
}

QVariantList PdfDocument::imagesVariant() const
{
    QVariantList l;
    l.reserve(imageCount());
    std::for_each(d->m_images.begin(), d->m_images.end(), [&l](const PdfImage& img) { l.push_back(QVariant::fromValue(img)); });
    return l;
}

int PdfDocument::pageCount() const
{
    return d->m_pages.size();
}

PdfPage PdfDocument::page(int index) const
{
    return d->m_pages[index];
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

    std::unique_ptr<ExtractorOutputDevice> device(new ExtractorOutputDevice(doc->d.get()));
    doc->d->m_pages.reserve(popplerDoc->getNumPages());
    for (int i = 0; i < popplerDoc->getNumPages(); ++i) {
        popplerDoc->displayPageSlice(device.get(), i + 1, 72, 72, 0, false, true, false, -1, -1, -1, -1);
        const auto pageRect = popplerDoc->getPage(i + 1)->getCropBox();
        std::unique_ptr<GooString> s(device->getText(pageRect->x1, pageRect->y1, pageRect->x2, pageRect->y2));
        std::copy(device->m_images.begin(), device->m_images.end(), std::back_inserter(doc->d->m_images));

        PdfPage page;
        page.d->m_pageNum = i;
        page.d->m_doc = doc->d.get();
        page.d->m_text = QString::fromUtf8(s->getCString());
        page.d->m_images = std::move(device->m_images);
        for (auto it = page.d->m_images.begin(); it != page.d->m_images.end(); ++it) {
            (*it).d->m_page = page.d.data();
        }
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
