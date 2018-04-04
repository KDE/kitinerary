/*
    Copyright (C) 2018 Luca Beltrame <lbeltrame@kde.org>

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


#ifndef KITINERARY_ORGANIZATION_H
#define KITINERARY_ORGANIZATION_H

#include "kitinerary_export.h"

#include "datatypes.h"
#include "place.h"

class QUrl;

namespace KItinerary {

class OrganizationPrivate;

/** An organization
 *  @see https://schema.org/Organization
 */

class KITINERARY_EXPORT Organization
{
    KITINERARY_GADGET(Organization)
    KITINERARY_PROPERTY(QString, name, setName)
    KITINERARY_PROPERTY(QString, email, setEmail)
    KITINERARY_PROPERTY(QString, telephone, setTelephone)
    KITINERARY_PROPERTY(QUrl, url, setUrl)
    KITINERARY_PROPERTY(KItinerary::PostalAddress, address, setAddress)
private:
    QExplicitlySharedDataPointer<OrganizationPrivate> d;
};

} // namespace KItinerary

Q_DECLARE_METATYPE(KItinerary::Organization)

#endif // KITINERARY_ORGANIZATION_H
