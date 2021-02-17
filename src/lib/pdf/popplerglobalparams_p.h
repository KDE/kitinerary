/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_POPPLERGLOBALPARAMS_P_H
#define KITINERARY_POPPLERGLOBALPARAMS_P_H

#include <memory>

class GlobalParams;

namespace KItinerary {

/** RAII wrapper of the globalParams object. */
class PopplerGlobalParams
{
public:
    PopplerGlobalParams();
    ~PopplerGlobalParams();

private:
    std::unique_ptr<GlobalParams> m_prev;
};

}

#endif // KITINERARY_POPPLERGLOBALPARAMS_P_H
