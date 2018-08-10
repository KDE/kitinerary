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

    Q_PROPERTY(bool isValid READ isValid STORED false)
public:
    /** Returns @c true if both latitude and longitude are set and within
     *  the valid range.
     */
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
    KITINERARY_PROPERTY(QString, addressRegion, setAddressRegion)
    /** The country this address is in, as ISO 3166-1 alpha 2 code. */
    KITINERARY_PROPERTY(QString, addressCountry, setAddressCountry)

    Q_PROPERTY(bool isEmpty READ isEmpty STORED false)
public:
    /** Returns @c true if there is no property set in this object. */
    bool isEmpty() const;
private:
    QExplicitlySharedDataPointer<PostalAddressPrivate> d;
};

class PlacePrivate;

/** Base class for places.
 *  @see https://schema.org/Place
 */
class KITINERARY_EXPORT Place
{
    KITINERARY_BASE_GADGET(Place)
    KITINERARY_PROPERTY(QString, name, setName)
    KITINERARY_PROPERTY(KItinerary::PostalAddress, address, setAddress)
    KITINERARY_PROPERTY(KItinerary::GeoCoordinates, geo, setGeo)
    /** Identifier.
     *  We use the following schemas currently:
     *  - 'uic:', UIC station code (see https://www.wikidata.org/wiki/Property:P722)
     *  - 'sncf:', Gares & Connextions ID, (see https://www.wikidata.org/wiki/Property:P3104), French train station identifier.
     *  - 'ibnr:', Internationale Bahnhofsnummer, (see https://www.wikidata.org/wiki/Property:P954), German train station identifier.
     *  @see http://schema.org/docs/datamodel.html#identifierBg
     */
    KITINERARY_PROPERTY(QString, identifier, setIdentifier)
protected:
    ///@cond internal
    QExplicitlySharedDataPointer<PlacePrivate> d;
    ///@endcond
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


/** Tourist attraction (e.g. Museum, sight, etc.).
 * @see https://schema.org/TouristAttraction
 */
class KITINERARY_EXPORT TouristAttraction: public Place
{
    KITINERARY_GADGET(TouristAttraction)
};

}

Q_DECLARE_METATYPE(KItinerary::GeoCoordinates)
Q_DECLARE_METATYPE(KItinerary::PostalAddress)
Q_DECLARE_METATYPE(KItinerary::Airport)
Q_DECLARE_METATYPE(KItinerary::TrainStation)
Q_DECLARE_METATYPE(KItinerary::BusStation)
Q_DECLARE_METATYPE(KItinerary::TouristAttraction)

#endif // KITINERARY_PLACE_H
