/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_POPPLERUTILS_P_H
#define KITINERARY_POPPLERUTILS_P_H

#include <qnamespace.h>

class GfxPath;
class GfxState;

class QBrush;
class QPainterPath;
class QPen;
class QTransform;

namespace KItinerary {

/** Utilities for interacting with Poppler. */
namespace PopplerUtils
{
    /** Returns the current pen. */
    QPen currentPen(GfxState *state);
    /** Retruns the current brush. */
    QBrush currentBrush(GfxState *state);
    /** Returns the current transformation matrix. */
    QTransform currentTransform(GfxState *state);

    /** Convets a Poppler path into a Qt path. */
    QPainterPath convertPath(const GfxPath *path, Qt::FillRule fillRule);
}

}

#endif // KITINERARY_POPPLERUTILS_P_H
