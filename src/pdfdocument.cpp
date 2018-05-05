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
    int m_ref = -1;
    QImage m_img;
    double m_x = NAN;
    double m_y = NAN;
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
#endif

}

#ifdef HAVE_POPPLER
ExtractorOutputDevice::ExtractorOutputDevice(PdfDocumentPrivate *dd)
    : TextOutputDev(nullptr, false, 0, false, false)
    , d(dd)
{
}

void ExtractorOutputDevice::drawImage(GfxState* state, Object* ref, Stream* str, int width, int height, GfxImageColorMap* colorMap, GBool interpolate, int* maskColors, GBool inlineImg)
{
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

    QImage img;
    if (colorMap->getColorSpace()->getMode() == csIndexed) {
        img = QImage(width, height, QImage::Format_RGB888);
    } else if (colorMap->getNumPixelComps() == 1 && (colorMap->getBits() >= 1 && colorMap->getBits() <= 8)) {
        img = QImage(width, height, QImage::Format_Grayscale8);
    } else if (colorMap->getNumPixelComps() == 3 && colorMap->getBits() == 8) {
        img = QImage(width, height, QImage::Format_RGB888);
    } else {
        return;
    }

    std::unique_ptr<ImageStream> imgStream(new ImageStream(str, width, colorMap->getNumPixelComps(), colorMap->getBits()));
    imgStream->reset();

    for (int i = 0; i < height; ++i) {
        const auto row = imgStream->getLine();
        switch (img.format()) {
            case QImage::Format_RGB888:
            {
                auto imgData = img.scanLine(i);
                GfxRGB rgb;
                for (int j = 0; j < width; ++j) {
                    colorMap->getRGB(row + (j * colorMap->getNumPixelComps()), &rgb);
                    *imgData++ = colToByte(rgb.r);
                    *imgData++ = colToByte(rgb.g);
                    *imgData++ = colToByte(rgb.b);
                }
                break;
            }
            case QImage::Format_Grayscale8:
            {
                auto imgData = img.scanLine(i);
                GfxGray gray;
                for (int j = 0; j < width; ++j) {
                    colorMap->getGray(row + j, &gray);
                    *imgData++ = colToByte(gray);
                }
                break;
            }
            default:
                break;
        }
    }

    imgStream->close();
    PdfImage pdfImg;
    pdfImg.d->m_img = img;
    if (ref->isRef()) {
        pdfImg.d->m_ref = ref->getRef().num;
    }
    pdfImg.d->m_x = state->getCTM()[4];
    pdfImg.d->m_y = state->getCTM()[5];
    m_images.push_back(pdfImg);
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
    return d->m_img.height();
}

int PdfImage::width() const
{
    return d->m_img.width();
}

QImage PdfImage::image() const
{
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
    GlobalParams params;
    QScopedValueRollback<GlobalParams*> globalParamResetter(globalParams, &params);

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
    GlobalParams params;
    QScopedValueRollback<GlobalParams*> globalParamResetter(globalParams, &params);
    const auto pageRect = d->m_doc->m_popplerDoc->getPage(d->m_pageNum + 1)->getCropBox();

    for (const auto &img : d->m_images) {
        if ((img.d->m_x >= ratio(pageRect->x1, pageRect->x2, left) && img.d->m_x <= ratio(pageRect->x1, pageRect->x2, right)) &&
            (img.d->m_y >= ratio(pageRect->y1, pageRect->y2, top)  && img.d->m_y <= ratio(pageRect->y1, pageRect->y2, bottom)))
        {
            l.push_back(QVariant::fromValue(img));
        }
    }
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
    GlobalParams params;
    QScopedValueRollback<GlobalParams*> globalParamResetter(globalParams, &params);

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
