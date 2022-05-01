/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

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

    bool needNonText() override { return true; }
    void drawImage(GfxState *state, Object *ref, Stream *str, int width, int height, GfxImageColorMap *colorMap, bool interpolate, PopplerMaskColors *maskColors, bool inlineImg) override;

    // operations used to detect vector barcodes
    void saveState(GfxState *state) override;
    void restoreState(GfxState *state) override;
    void stroke(GfxState *state) override;
    void fill(GfxState *state) override;
    void eoFill(GfxState *state) override;

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
};

}

