/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

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
    /** Returns the current brush. */
    QBrush currentBrush(GfxState *state);
    /** Returns the current transformation matrix. */
    QTransform currentTransform(GfxState *state);

    /** Converts a Poppler path into a Qt path. */
    QPainterPath convertPath(const GfxPath *path, Qt::FillRule fillRule);
}

}

