/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "pdfextractoroutputdevice_p.h"
#include "pdfbarcodeutil_p.h"
#include "pdfimage.h"
#include "pdfimage_p.h"
#include "popplerutils_p.h"

#include <QDebug>

using namespace KItinerary;

PdfExtractorOutputDevice::PdfExtractorOutputDevice()
    : TextOutputDev(nullptr, false, 0, false, false)
{
}

void PdfExtractorOutputDevice::drawImage(GfxState* state, Object* ref, Stream* str, int width, int height, GfxImageColorMap* colorMap, bool interpolate, PopplerMaskColors* maskColors, bool inlineImg)
{
    Q_UNUSED(str)
    Q_UNUSED(interpolate)
    Q_UNUSED(maskColors)
    Q_UNUSED(inlineImg)

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

#if KPOPPLER_VERSION >= QT_VERSION_CHECK(0, 69, 0)
    pdfImg.d->m_colorMap.reset(colorMap->copy());
#endif
    pdfImg.d->m_sourceHeight = height;
    pdfImg.d->m_sourceWidth = width;
    pdfImg.d->m_width = width;
    pdfImg.d->m_height = height;
    // deal with aspect-ratio changing scaling
    const auto sourceAspectRatio = (double)width / (double)height;
    const auto targetAspectRatio = std::abs(state->getCTM()[0] / -state->getCTM()[3]);
    if (!qFuzzyCompare(sourceAspectRatio, targetAspectRatio) && qFuzzyIsNull(state->getCTM()[1]) && qFuzzyIsNull(state->getCTM()[2])) {
        if (targetAspectRatio > sourceAspectRatio) {
            pdfImg.d->m_width = width * targetAspectRatio / sourceAspectRatio;
        } else {
            pdfImg.d->m_height = height * sourceAspectRatio / targetAspectRatio;
        }
    }
    pdfImg.d->m_transform = PopplerUtils::currentTransform(state);
    pdfImg.d->m_format = format;
    m_images.push_back(pdfImg);
}

void PdfExtractorOutputDevice::saveState(GfxState *state)
{
    Q_UNUSED(state)
    m_vectorOps.push_back(VectorOp{VectorOp::PushState, {}, {}});
}

void PdfExtractorOutputDevice::restoreState(GfxState *state)
{
    Q_UNUSED(state)
    if (m_vectorOps.empty()) {
        return;
    }
    const auto &lastOp = *(m_vectorOps.end() -1);
    if (lastOp.type == VectorOp::PushState) {
        m_vectorOps.resize(m_vectorOps.size() - 1);
    } else {
        m_vectorOps.push_back(VectorOp{VectorOp::PopState, {}, {}});
    }
}

static bool isRelevantStroke(const QPen &pen)
{
    return !qFuzzyCompare(pen.widthF(), 0.0) && pen.color() == Qt::black;
}

static bool isRectangularPath(const QPainterPath &path)
{
    qreal x = 0.0, y = 0.0;
    for (int i = 0; i < path.elementCount(); ++i) {
        const auto elem = path.elementAt(i);
        switch (elem.type) {
            case QPainterPath::MoveToElement:
                x = elem.x;
                y = elem.y;
                break;
            case QPainterPath::LineToElement:
                if (x != elem.x && y != elem.y) {
                    qDebug() << "path contains diagonal line, discarding";
                    return false;
                }
                x = elem.x;
                y = elem.y;
                break;
            case QPainterPath::CurveToElement:
            case QPainterPath::CurveToDataElement:
                qDebug() << "path contains a curve, discarding";
                return false;
        }
    }

    return true;
}

void PdfExtractorOutputDevice::stroke(GfxState *state)
{
    const auto pen = PopplerUtils::currentPen(state);
    if (!isRelevantStroke(pen)) {
        return;
    }

    const auto path = PopplerUtils::convertPath(state->getPath(), Qt::WindingFill);
    if (!isRectangularPath(path)) {
        return;
    }
    const auto t = PopplerUtils::currentTransform(state);
    m_vectorOps.push_back(VectorOp{VectorOp::Path, t, {path, pen, QBrush()}});
}

static bool isRelevantFill(const QBrush &brush)
{
    return brush.color() == Qt::black;
}

void PdfExtractorOutputDevice::fill(GfxState *state)
{
    const auto brush = PopplerUtils::currentBrush(state);
    if (!isRelevantFill(brush)) {
        return;
    }

    const auto path = PopplerUtils::convertPath(state->getPath(), Qt::WindingFill);
    const auto b = path.boundingRect();
    if (b.width() == 0  || b.height() == 0) {
        return;
    }

    const auto t = PopplerUtils::currentTransform(state);
    m_vectorOps.push_back(VectorOp{VectorOp::Path, t, {path, QPen(), brush}});
}

void PdfExtractorOutputDevice::eoFill(GfxState *state)
{
    const auto brush = PopplerUtils::currentBrush(state);
    if (!isRelevantFill(brush)) {
        return;
    }

    const auto path = PopplerUtils::convertPath(state->getPath(), Qt::OddEvenFill);
    const auto b = path.boundingRect();
    if (b.width() == 0  || b.height() == 0) {
        return;
    }

    const auto t = PopplerUtils::currentTransform(state);
    m_vectorOps.push_back(VectorOp{VectorOp::Path, t, {path, QPen(), brush}});
}

void PdfExtractorOutputDevice::finalize()
{
    // remove single state groups, then try to merge adjacents paths
    std::vector<VectorOp> mergedOps;
    mergedOps.reserve(m_vectorOps.size());
    for (auto it = m_vectorOps.begin(); it != m_vectorOps.end(); ++it) {
        if ((*it).type == VectorOp::PushState && std::distance(it, m_vectorOps.end()) >= 2 && (*(it + 1)).type == VectorOp::Path && (*(it + 2)).type == VectorOp::PopState) {
            ++it;
            mergedOps.push_back(*it);
            ++it;
        } else {
            mergedOps.push_back(*it);
        }
    }
    //qDebug() << m_vectorOps.size() << mergedOps.size();

    std::vector<PdfVectorPicture::PathStroke> strokes;
    QTransform t;
    for (const auto &op : mergedOps) {
        if (op.type == VectorOp::Path) {
            if (t.isIdentity()) {
                t = op.transform;
            }
            if (t != op.transform) {
                //qDebug() << "diffent transforms for strokes, not supported yet";
                continue;
            }
            strokes.push_back(op.stroke);
        } else if (!strokes.empty()) {
            PdfVectorPicture pic;
            pic.setStrokes(std::move(strokes));
            pic.setTransform(t);
            addVectorImage(pic);
            t = QTransform();
        }
    }
    if (!strokes.empty()) {
        PdfVectorPicture pic;
        pic.setStrokes(std::move(strokes));
        pic.setTransform(t);
        addVectorImage(pic);
    }
}

void PdfExtractorOutputDevice::addVectorImage(const PdfVectorPicture &pic)
{
    if (PdfBarcodeUtil::isPlausiblePath(pic.pathElementsCount(), BarcodeDecoder::Any) == BarcodeDecoder::None) {
        return;
    }

    PdfImage img;
    img.d->m_height = pic.height();
    img.d->m_width = pic.width();
    img.d->m_sourceHeight = pic.sourceHeight();
    img.d->m_sourceWidth = pic.sourceWidth();
    img.d->m_transform = pic.transform();
    img.d->m_vectorPicture = pic;
    m_images.push_back(img);
}
