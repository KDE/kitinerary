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

using namespace KItinerary;

namespace KItinerary {
class PdfDocumentPrivate {
public:
    QByteArray m_pdfData;
    QString m_text;
    std::vector<QImage> m_images;
    std::vector<int> m_imgRefs;
};

#ifdef HAVE_POPPLER
class ExtractorOutputDevice : public TextOutputDev
{
public:
    ExtractorOutputDevice(PdfDocumentPrivate *dd);
    GBool needNonText() override { return true; }
    void drawImage(GfxState *state, Object *ref, Stream *str, int width, int height, GfxImageColorMap *colorMap, GBool interpolate, int *maskColors, GBool inlineImg) override;

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
    Q_UNUSED(state);
    Q_UNUSED(interpolate);
    Q_UNUSED(maskColors);
    Q_UNUSED(inlineImg);

    if (!colorMap || !colorMap->isOk()) {
        return;
    }

    // check for duplicate occurances of ref->getRef().num
    if (ref->isRef()) {
        auto it = std::lower_bound(d->m_imgRefs.begin(), d->m_imgRefs.end(), ref->getRef().num);
        if (it != d->m_imgRefs.end() && *it == ref->getRef().num) {
            return;
        }
        d->m_imgRefs.insert(it, ref->getRef().num);
    }

    QImage img;
    switch (colorMap->getColorSpace()->getMode()) {
        case csCalGray:
        case csDeviceGray:
            if (colorMap->getNumPixelComps() == 1 && colorMap->getBits() == 1) {
                img = QImage(width, height, QImage::Format_Grayscale8);
            } else if (colorMap->getNumPixelComps() == 1 && colorMap->getBits() == 8) {
                img = QImage(width, height, QImage::Format_Grayscale8);
            } else {
                return;
            }
            break;
        case csCalRGB:
        case csDeviceRGB:
        case csICCBased:
            if (colorMap->getNumPixelComps() == 3 && colorMap->getBits() == 8) {
                img = QImage(width, height, QImage::Format_RGB888);
            } else {
                return;
            }
            break;
        default:
            // the fancy photo/print formats aren't used for the stuff we are interested in
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
                    colorMap->getRGB(row + (j * 3), &rgb);
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
            case QImage::Format_Mono:
            {
                GfxGray gray;
                for (int j = 0; j < width; ++j) {
                    colorMap->getGray(row + j, &gray);
                    img.setPixel(j, i, colToByte(gray));
                }
                break;
            }
            default:
                break;
        }
    }

    imgStream->close();
    d->m_images.push_back(img);
}
#endif

PdfDocument::PdfDocument(QObject *parent)
    : QObject(parent)
    , d(new PdfDocumentPrivate)
{
}

PdfDocument::~PdfDocument() = default;

QString PdfDocument::text() const
{
    return d->m_text;
}

int PdfDocument::imageCount() const
{
    return d->m_images.size();
}

QImage PdfDocument::image(int index) const
{
    return d->m_images[index];
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
    for (int i = 0; i < popplerDoc->getNumPages(); ++i) {
        popplerDoc->displayPageSlice(device.get(), i + 1, 72, 72, 0, false, true, false, -1, -1, -1, -1);
        auto pageRect = popplerDoc->getPage(i + 1)->getCropBox();
        std::unique_ptr<GooString> s(device->getText(pageRect->x1, pageRect->y1, pageRect->x2, pageRect->y2));
        doc->d->m_text += QString::fromUtf8(s->getCString());
    }

    return doc.release();
#else
    Q_UNUSED(data);
    Q_UNUSED(parent);
    return nullptr;
#endif
}
