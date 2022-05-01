/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <config-kitinerary.h>
#include <qglobal.h>

#if KPOPPLER_VERSION >= QT_VERSION_CHECK(0, 82, 0)
using PopplerMaskColors = const int;
#else
using PopplerMaskColors = int;
#endif
