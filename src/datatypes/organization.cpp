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

#include "organization.h"
#include "datatypes_p.h"

#include <QUrl>

using namespace KItinerary;

namespace KItinerary {

class OrganizationPrivate: public QSharedData
{
    KITINERARY_PRIVATE_BASE_GADGET(Organization)
public:
    QString name;
    QString email;
    QString telephone;
    QUrl url;
    PostalAddress address;
    GeoCoordinates geo;
};

KITINERARY_MAKE_BASE_CLASS(Organization)
KITINERARY_MAKE_PROPERTY(Organization, QString, name, setName)
KITINERARY_MAKE_PROPERTY(Organization, QString, email, setEmail)
KITINERARY_MAKE_PROPERTY(Organization, QString, telephone, setTelephone)
KITINERARY_MAKE_PROPERTY(Organization, QUrl, url, setUrl)
KITINERARY_MAKE_PROPERTY(Organization, PostalAddress, address, setAddress)
KITINERARY_MAKE_PROPERTY(Organization, KItinerary::GeoCoordinates, geo, setGeo)
KITINERARY_MAKE_OPERATOR(Organization)

class AirlinePrivate : public OrganizationPrivate
{
    KITINERARY_PRIVATE_GADGET(Airline)
public:
    QString iataCode;
};

KITINERARY_MAKE_SUB_CLASS(Airline, Organization)
KITINERARY_MAKE_PROPERTY(Airline, QString, iataCode, setIataCode)
KITINERARY_MAKE_OPERATOR(Airline)

class FoodEstablishmentPrivate: public OrganizationPrivate
{
    KITINERARY_PRIVATE_GADGET(FoodEstablishment)
};
KITINERARY_MAKE_SUB_CLASS(FoodEstablishment, Organization)
KITINERARY_MAKE_OPERATOR(FoodEstablishment)

class LodgingBusinessPrivate : public OrganizationPrivate
{
    KITINERARY_PRIVATE_GADGET(LodgingBusiness)
};
KITINERARY_MAKE_SUB_CLASS(LodgingBusiness, Organization)
KITINERARY_MAKE_OPERATOR(LodgingBusiness)

}

template <>
KItinerary::OrganizationPrivate *QExplicitlySharedDataPointer<KItinerary::OrganizationPrivate>::clone()
{
    return d->clone();
}

#include "moc_organization.cpp"
