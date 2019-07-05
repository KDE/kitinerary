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

#include <config-kitinerary.h>

#include "popplerutils_p.h"

#include <QBrush>
#include <QPainterPath>
#include <QPen>

#include <memory>

#ifdef HAVE_POPPLER

#include <GfxState.h>
#include <GlobalParams.h>

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

QPainterPath PopplerUtils::convertPath(GfxPath *path, Qt::FillRule fillRule)
{
    QPainterPath qpp;
    qpp.setFillRule(fillRule);

    for (auto i = 0; i < path->getNumSubpaths(); ++i) {
        const auto subpath = path->getSubpath(i);
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

GlobalParams* PopplerUtils::globalParams()
{
    static std::unique_ptr<GlobalParams> s_globalParams;
    if (!s_globalParams) {
        s_globalParams.reset(new GlobalParams);
    }
    return s_globalParams.get();
}

#endif
