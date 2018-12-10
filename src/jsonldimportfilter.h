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
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef KITINERARY_JSONLDIMPORTFILTER_H
#define KITINERARY_JSONLDIMPORTFILTER_H

class QJsonObject;

namespace KItinerary {

/** Filter input JSON for loading with JsonLdDocument, to deal with obsolete
 *  or renamed elements.
 */
namespace JsonLdImportFilter
{
    /** Filter the top-level object @p obj for loading with JsonLdDocument. */
    QJsonObject filterObject(const QJsonObject &obj);
}

}

#endif // KITINERARY_JSONLDIMPORTFILTER_H
