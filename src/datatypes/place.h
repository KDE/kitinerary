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

#ifndef KITINERARY_PLACE_H
#define KITINERARY_PLACE_H

#include "kitinerary_export.h"

#include "datatypes.h"

class QVariant;

namespace KItinerary {

class GeoCoordinatesPrivate;

/** Geographic coordinates.
 *  @see https://schema.org/GeoCoordinates
 */
class KITINERARY_EXPORT GeoCoordinates
{
    KITINERARY_GADGET(GeoCoordinates)
    KITINERARY_PROPERTY(float, latitude, setLatitude)
    KITINERARY_PROPERTY(float, longitude, setLongitude)
public:
    bool isValid() const;
private:
    QExplicitlySharedDataPointer<GeoCoordinatesPrivate> d;
};

class PostalAddressPrivate;

/** Postal address.
 *  @see https://schema.org/PostalAddress
 */
class KITINERARY_EXPORT PostalAddress
{
    KITINERARY_GADGET(PostalAddress)
    KITINERARY_PROPERTY(QString, streetAddress, setStreeAddress)
    KITINERARY_PROPERTY(QString, addressLocality, setAddressLocality)
    KITINERARY_PROPERTY(QString, postalCode, setPostalCode)
    KITINERARY_PROPERTY(QString, addressCountry, setAddressCountry)
private:
    QExplicitlySharedDataPointer<PostalAddressPrivate> d;
};

class PlacePrivate;

/** Base class for places.
 *  @see https://schema.org/Place
 */
class KITINERARY_EXPORT Place
{
    KITINERARY_ABSTRACT_GADGET(Place)
    KITINERARY_PROPERTY(QString, name, setName)
    KITINERARY_PROPERTY(KItinerary::PostalAddress, address, setAddress)
    KITINERARY_PROPERTY(KItinerary::GeoCoordinates, geo, setGeo)
protected:
    QExplicitlySharedDataPointer<PlacePrivate> d;
};

/** Airport.
 *  @see https://schema.org/Airport.
 */
class KITINERARY_EXPORT Airport : public Place
{
    KITINERARY_GADGET(Airport)
    KITINERARY_PROPERTY(QString, iataCode, setIataCode)
};

/** Train station.
 *  @see https://schema.org/TrainStation
 */
class KITINERARY_EXPORT TrainStation : public Place
{
    KITINERARY_GADGET(TrainStation)
};

/** Bus station.
 *  @see https://schema.org/BusStation
 */
class KITINERARY_EXPORT BusStation : public Place
{
    KITINERARY_GADGET(BusStation)
};

/** Hotel.
 *  @see https://schema.org/LodgingBusiness
 */
class KITINERARY_EXPORT LodgingBusiness: public Place
{
    KITINERARY_GADGET(LodgingBusiness)
};

}

Q_DECLARE_METATYPE(KItinerary::GeoCoordinates)
Q_DECLARE_METATYPE(KItinerary::PostalAddress)
Q_DECLARE_METATYPE(KItinerary::Airport)
Q_DECLARE_METATYPE(KItinerary::TrainStation)
Q_DECLARE_METATYPE(KItinerary::BusStation)
Q_DECLARE_METATYPE(KItinerary::LodgingBusiness)

#endif // KITINERARY_PLACE_H
