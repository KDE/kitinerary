/*
    SPDX-FileCopyrightText: 2018 Luca Beltrame <lbeltrame@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "organization.h"
#include "datatypes_p.h"
#include "locationutil.h"

#include <QUrl>

using namespace KItinerary;

namespace KItinerary {

class OrganizationPrivate: public QSharedData
{
    KITINERARY_PRIVATE_BASE_GADGET(Organization)
public:
    QString name;
    QString identifier;
    QString description;
    QUrl image;
    QUrl logo;
    QString email;
    QString telephone;
    QUrl url;
    PostalAddress address;
    GeoCoordinates geo;
    QVariantList potentialAction;
};

KITINERARY_MAKE_BASE_CLASS(Organization)
KITINERARY_MAKE_PROPERTY(Organization, QString, name, setName)
KITINERARY_MAKE_PROPERTY(Organization, QString, identifier, setIdentifier)
KITINERARY_MAKE_PROPERTY(Organization, QString, description, setDescription)
KITINERARY_MAKE_PROPERTY(Organization, QUrl, image, setImage)
KITINERARY_MAKE_PROPERTY(Organization, QUrl, logo, setLogo)
KITINERARY_MAKE_PROPERTY(Organization, QString, email, setEmail)
KITINERARY_MAKE_PROPERTY(Organization, QString, telephone, setTelephone)
KITINERARY_MAKE_PROPERTY(Organization, QUrl, url, setUrl)
KITINERARY_MAKE_PROPERTY(Organization, PostalAddress, address, setAddress)
KITINERARY_MAKE_PROPERTY(Organization, KItinerary::GeoCoordinates, geo, setGeo)
KITINERARY_MAKE_PROPERTY(Organization, QVariantList, potentialAction, setPotentialAction)
KITINERARY_MAKE_OPERATOR(Organization)

QUrl Organization::geoUri() const
{
    return LocationUtil::geoUri(*this);
}

class AirlinePrivate : public OrganizationPrivate
{
    KITINERARY_PRIVATE_GADGET(Airline)
public:
    QString iataCode;
};

KITINERARY_MAKE_SUB_CLASS(Airline, Organization)
KITINERARY_MAKE_PROPERTY(Airline, QString, iataCode, setIataCode)
KITINERARY_MAKE_OPERATOR(Airline)

class LocalBusinessPrivate : public OrganizationPrivate
{
    KITINERARY_PRIVATE_GADGET(LocalBusiness)
};
KITINERARY_MAKE_INTERMEDIATE_CLASS(LocalBusiness, Organization)
KITINERARY_MAKE_OPERATOR(LocalBusiness)

class FoodEstablishmentPrivate: public LocalBusinessPrivate
{
    KITINERARY_PRIVATE_GADGET(FoodEstablishment)
};
KITINERARY_MAKE_SUB_CLASS(FoodEstablishment, LocalBusiness)
KITINERARY_MAKE_OPERATOR(FoodEstablishment)

class LodgingBusinessPrivate : public LocalBusinessPrivate
{
    KITINERARY_PRIVATE_GADGET(LodgingBusiness)
};
KITINERARY_MAKE_SUB_CLASS(LodgingBusiness, LocalBusiness)
KITINERARY_MAKE_OPERATOR(LodgingBusiness)

}

template <>
KItinerary::OrganizationPrivate *QExplicitlySharedDataPointer<KItinerary::OrganizationPrivate>::clone()
{
    return d->clone();
}

#include "moc_organization.cpp"
