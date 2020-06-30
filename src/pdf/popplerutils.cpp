/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <config-kitinerary.h>

#include "popplerutils_p.h"

#include <QBrush>
#include <QPainterPath>
#include <QPen>

#include <memory>

#ifdef HAVE_POPPLER

#include <GfxState.h>

using namespace KItinerary;

QPen PopplerUtils::currentPen(GfxState *state)
{
    QPen pen;
    pen.setStyle(Qt::SolidLine);
    pen.setWidthF(state->getLineWidth());

    GfxRGB rgb;
    state->getStrokeRGB(&rgb);
    QColor c;
    c.setRgbF(colToDbl(rgb.r), colToDbl(rgb.g), colToDbl(rgb.b), 1.0f);
    pen.setColor(c);

    switch (state->getLineCap()) {
        case 0: pen.setCapStyle(Qt::FlatCap); break;
        case 1: pen.setCapStyle(Qt::RoundCap); break;
        case 2: pen.setCapStyle(Qt::SquareCap); break;
    }

    switch (state->getLineJoin()) {
        case 0: pen.setJoinStyle(Qt::SvgMiterJoin); break;
        case 1: pen.setJoinStyle(Qt::RoundJoin); break;
        case 2: pen.setJoinStyle(Qt::BevelJoin); break;
    }

    pen.setMiterLimit(state->getMiterLimit());

    return pen;
}

QBrush PopplerUtils::currentBrush(GfxState* state)
{
    QBrush brush;
    brush.setStyle(Qt::SolidPattern);

    GfxRGB rgb;
    state->getFillRGB(&rgb);
    QColor c;
    c.setRgbF(colToDbl(rgb.r), colToDbl(rgb.g), colToDbl(rgb.b), 1.0f);
    brush.setColor(c);

    return brush;
}

QTransform KItinerary::PopplerUtils::currentTransform(GfxState *state)
{
    const auto ctm = state->getCTM();
    return QTransform(ctm[0], ctm[1], ctm[2], ctm[3], ctm[4], ctm[5]);
}

QPainterPath PopplerUtils::convertPath(const GfxPath *path, Qt::FillRule fillRule)
{
    QPainterPath qpp;
    qpp.setFillRule(fillRule);

    for (auto i = 0; i < path->getNumSubpaths(); ++i) {
#if KPOPPLER_VERSION >= QT_VERSION_CHECK(0, 83, 0)
        const auto subpath = path->getSubpath(i);
#else
        const auto subpath = const_cast<GfxPath*>(path)->getSubpath(i);
#endif
        if (subpath->getNumPoints() > 0) {
            qpp.moveTo(subpath->getX(0), subpath->getY(0));
            for (auto j = 1;j < subpath->getNumPoints();) {
                if (subpath->getCurve(j)) {
                    qpp.cubicTo(subpath->getX(j),   subpath->getY(j), subpath->getX(j+1), subpath->getY(j+1), subpath->getX(j+2), subpath->getY(j+2));
                    j += 3;
                } else {
                    qpp.lineTo(subpath->getX(j), subpath->getY(j));
                    ++j;
                }
            }
            if (subpath->isClosed()) {
                qpp.closeSubpath();
            }
        }
    }
    return qpp;
}

#endif
