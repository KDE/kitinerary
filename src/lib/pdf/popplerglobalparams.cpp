/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "config-kitinerary.h"
#include "popplerglobalparams_p.h"

#include <qglobal.h>

#include <GlobalParams.h>

#include <memory>

using namespace KItinerary;

static std::unique_ptr<GlobalParams> s_globalParams;

PopplerGlobalParams::PopplerGlobalParams()
{
    if (!s_globalParams) {
        s_globalParams = std::make_unique<GlobalParams>();
    }

#if KPOPPLER_VERSION <= QT_VERSION_CHECK(0, 82, 0)
    m_prev.reset(globalParams);
    globalParams = s_globalParams.get();
#else
    std::swap(globalParams, m_prev);
    std::swap(s_globalParams, globalParams);
#endif
}

PopplerGlobalParams::~PopplerGlobalParams()
{
#if KPOPPLER_VERSION <= QT_VERSION_CHECK(0, 82, 0)
    globalParams = m_prev.release();
#else
    std::swap(s_globalParams, globalParams);
    std::swap(globalParams, m_prev);
#endif
}
