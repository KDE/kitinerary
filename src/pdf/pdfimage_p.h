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

#ifndef KITINERARY_PDFIMAGE_P_H
#define KITINERARY_PDFIMAGE_P_H

#include <config-kitinerary.h>

#include "pdfimage.h"
#include "pdfvectorpicture_p.h"

#include <QImage>
#include <QSharedData>
#include <QTransform>

#include <memory>

class GfxImageColorMap;
class Stream;

namespace KItinerary {

class PdfPagePrivate;

class PdfImagePrivate : public QSharedData {
public:
    QImage load();
#ifdef HAVE_POPPLER
    QImage load(Stream *str, GfxImageColorMap *colorMap);
#endif

    // pixel data
    int m_refNum = -1;
    int m_refGen = -1;
    QImage::Format m_format = QImage::Format_Invalid;
    PdfPagePrivate *m_page = nullptr;
#ifdef HAVE_POPPLER
    std::unique_ptr<GfxImageColorMap> m_colorMap;
#endif

    // vector data
    PdfVectorPicture m_vectorPicture;

    // common
    QTransform m_transform;
    int m_width = 0;
    int m_height = 0;
    int m_sourceWidth = 0;
    int m_sourceHeight = 0;
    PdfImage::LoadingHints m_loadingHints = PdfImage::NoHint;
};

}

#endif
