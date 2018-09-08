/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stringutil.h"

#include <QDebug>
#include <QString>

using namespace KItinerary;

QString StringUtil::normalize(const QString &str)
{
    QString out;
    out.reserve(str.size());
    for (const auto chr : str) {
        // case folding
        auto c = chr.toCaseFolded();

        // if the character has a canonical decomposition use that and skip the
        // combining diacritic markers following it
        if (c.decompositionTag() == QChar::Canonical) {
            out.push_back(c.decomposition().at(0));
        } else {
            out.push_back(c);
        }
    }
    return out;
}
