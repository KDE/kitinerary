/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_POPPLERTYPES_P_H
#define KITINERARY_POPPLERTYPES_P_H

#include <config-kitinerary.h>
#include <qglobal.h>

#ifdef HAVE_POPPLER
#if KPOPPLER_VERSION >= QT_VERSION_CHECK(0, 82, 0)
using PopplerMaskColors = const int;
#else
using PopplerMaskColors = int;
#endif
#endif

#endif // KITINERARY_POPPLERTYPES_P_H
