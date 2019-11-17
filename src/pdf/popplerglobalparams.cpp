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

#include "config-kitinerary.h"
#include "popplerglobalparams_p.h"

#include <qglobal.h>

#ifdef HAVE_POPPLER
#include <GlobalParams.h>

using namespace KItinerary;

static std::unique_ptr<GlobalParams> s_globalParams;

PopplerGlobalParams::PopplerGlobalParams()
{
    if (!s_globalParams) {
        s_globalParams.reset(new GlobalParams);
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

#endif
