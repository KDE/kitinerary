/*
    SPDX-FileCopyrightText: 2018 Luca Beltrame <lbeltrame@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/


#ifndef KITINERARY_ORGANIZATION_H
#define KITINERARY_ORGANIZATION_H

#include "kitinerary_export.h"

#include "datatypes.h"
#include "place.h"

class QUrl;

namespace KItinerary {

class OrganizationPrivate;

/** An organization.
 *
 *  This slightly deviates from the schema.org definition and also includes
 *  properties of Place that its sub-classes need. This is a simplification
 *  to avoid having to use multi-inheritance.
 *
 *  @see https://schema.org/Organization
 */

class KITINERARY_EXPORT Organization
{
    KITINERARY_BASE_GADGET(Organization)
    KITINERARY_PROPERTY(QString, name, setName)
    KITINERARY_PROPERTY(QString, description, setDescription)
    KITINERARY_PROPERTY(QUrl, image, setImage)
    KITINERARY_PROPERTY(QUrl, logo, setLogo)
    KITINERARY_PROPERTY(QString, email, setEmail)
    KITINERARY_PROPERTY(QString, telephone, setTelephone)
    KITINERARY_PROPERTY(QUrl, url, setUrl)
    KITINERARY_PROPERTY(KItinerary::PostalAddress, address, setAddress)
    KITINERARY_PROPERTY(KItinerary::GeoCoordinates, geo, setGeo)
    KITINERARY_PROPERTY(QVariantList, potentialAction, setPotentialAction)
protected:
    ///@cond internal
    QExplicitlySharedDataPointer<OrganizationPrivate> d;
    ///@endcond
};

class AirlinePrivate;

/** An airline.
 *  @see https://schema.org/Airline
 */
class KITINERARY_EXPORT Airline : public Organization
{
    KITINERARY_BASE_GADGET(Airline)
    KITINERARY_PROPERTY(QString, iataCode, setIataCode)
};

class LocalBusinessPrivate;

/** LocalBusiness.
 *  @see https://schema.org/LocalBusiness
 */
class KITINERARY_EXPORT LocalBusiness : public Organization
{
    KITINERARY_BASE_GADGET(LocalBusiness)
};

/** Hotel.
 *  @see https://schema.org/LodgingBusiness
 */
class KITINERARY_EXPORT LodgingBusiness: public LocalBusiness
{
    KITINERARY_GADGET(LodgingBusiness)
};

/** Food-related business (such as a restaurant, or a bakery).
 * @see https://schema.org/FoodEstablishment
 */
class KITINERARY_EXPORT FoodEstablishment: public LocalBusiness
{
    KITINERARY_GADGET(FoodEstablishment)
};

} // namespace KItinerary

Q_DECLARE_METATYPE(KItinerary::Organization)
Q_DECLARE_METATYPE(KItinerary::Airline)
Q_DECLARE_METATYPE(KItinerary::FoodEstablishment)
Q_DECLARE_METATYPE(KItinerary::LodgingBusiness)

#endif // KITINERARY_ORGANIZATION_H
