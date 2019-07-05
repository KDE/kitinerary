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

#ifndef KITINERARY_PDFEXTRACTOROUTPUTDEVICE_P_H
#define KITINERARY_PDFEXTRACTOROUTPUTDEVICE_P_H

#include <config-kitinerary.h>

#include "pdfvectorpicture_p.h"

#ifdef HAVE_POPPLER
#include <TextOutputDev.h>
#endif

#include <vector>

namespace KItinerary {

class PdfImage;
class PdfVectorPicture;

#ifdef HAVE_POPPLER
class PdfExtractorOutputDevice : public TextOutputDev
{
public:
    explicit PdfExtractorOutputDevice();

    // call once displaying has been completed
    void finalize();

    bool needNonText() override { return true; }
    void drawImage(GfxState *state, Object *ref, Stream *str, int width, int height, GfxImageColorMap *colorMap, bool interpolate, int *maskColors, bool inlineImg) override;

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

#endif

}

#endif // KITINERARY_PDFEXTRACTOROUTPUTDEVICE_H
