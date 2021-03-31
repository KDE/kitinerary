/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

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

