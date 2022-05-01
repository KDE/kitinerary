/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

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
    QImage load(Stream *str, GfxImageColorMap *colorMap);

    // pixel data
    int m_refNum = -1;
    int m_refGen = -1;
    QImage::Format m_format = QImage::Format_Invalid;
    PdfPagePrivate *m_page = nullptr;
    std::unique_ptr<GfxImageColorMap> m_colorMap;

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

