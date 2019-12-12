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
