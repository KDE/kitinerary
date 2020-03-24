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

#ifndef EXTRACTORVALIDATOR_H
#define EXTRACTORVALIDATOR_H

class QVariant;

/**
 * Validates extractor results.
 * Used to discard incomplete or otherwise invalid data.
 */
namespace ExtractorValidator
{
    /** Checks if the given element is valid.
     *  This will accept both Reservation object and things
     *  that can be reserved as top-level objects.
     */
    bool isValidElement(const QVariant &elem);
}

#endif // EXTRACTORVALIDATOR_H
