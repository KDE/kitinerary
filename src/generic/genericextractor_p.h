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

#ifndef KITINERARY_GENERICEXTRACTOR_H
#define KITINERARY_GENERICEXTRACTOR_H

#include <QJsonArray>
#include <QVariant>

namespace KItinerary {

/** Shared bits between all generic extractors. */
namespace GenericExtractor
{
    /** Generic extraction result. */
    struct Result {
        QJsonArray result; // JSON-LD data extracted from this document or page
        QVariant barcode; // unrecognized barcode for further processing
        int pageNum = -1; // page number, if result is from a single PDF page
    };
}

}

#endif // KITINERARY_GENERICEXTRACTOR_H
