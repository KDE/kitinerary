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
