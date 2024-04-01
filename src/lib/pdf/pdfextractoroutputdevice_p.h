/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "pdfimage.h"
#include "pdflink.h"
#include "pdfvectorpicture_p.h"
#include "popplertypes_p.h"

#include <TextOutputDev.h>

#include <vector>

namespace KItinerary {

class PdfImage;
class PdfVectorPicture;

class PdfExtractorOutputDevice : public TextOutputDev
{
public:
    explicit PdfExtractorOutputDevice();

    // call once displaying has been completed
    void finalize();

    // raster image operations
    bool needNonText() override { return true; }
    void drawImageMask(GfxState *state, Object *ref, Stream *str, int width, int height, bool invert, bool interpolate, bool inlineImg) override;
    void drawImage(GfxState *state, Object *ref, Stream *str, int width, int height, GfxImageColorMap *colorMap, bool interpolate, PopplerMaskColors *maskColors, bool inlineImg) override;
    void drawMaskedImage(GfxState *state, Object *ref, Stream *str, int width, int height, GfxImageColorMap *colorMap, bool interpolate, Stream *maskStr, int maskWidth, int maskHeight, bool maskInvert, bool maskInterpolate) override;
    void drawSoftMaskedImage(GfxState *state, Object *ref, Stream *str, int width, int height, GfxImageColorMap *colorMap, bool interpolate, Stream *maskStr, int maskWidth, int maskHeight, GfxImageColorMap *maskColorMap, bool maskInterpolate) override;

    // operations used to detect vector barcodes
    void saveState(GfxState *state) override;
    void restoreState(GfxState *state) override;
    void stroke(GfxState *state) override;
    void fill(GfxState *state) override;
    void eoFill(GfxState *state) override;

    // links
    void processLink(AnnotLink *link) override;

    void addRasterImage(GfxState *state, Object *ref, Stream *str, int width, int height, GfxImageColorMap *colorMap, PdfImageType type);
    void addVectorImage(const PdfVectorPicture &pic);

    // extracted images
    std::vector<PdfImage> m_images;

    // intermediate vector state
    struct VectorOp {
        enum { Path, PushState, PopState } type;
        QTransform transform;
        PdfVectorPicture::PathStroke stroke;
    };
    std::vector<VectorOp> m_vectorOps;

    // extracted links
    std::vector<PdfLink> m_links;
};

}

