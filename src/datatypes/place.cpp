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

#include "place.h"
#include "datatypes_p.h"

#include <cmath>

using namespace KItinerary;

namespace KItinerary {

class GeoCoordinatesPrivate : public QSharedData
{
public:
    float latitude = NAN;
    float longitude = NAN;
};

KITINERARY_MAKE_SIMPLE_CLASS(GeoCoordinates)
KITINERARY_MAKE_PROPERTY(GeoCoordinates, float, latitude, setLatitude)
KITINERARY_MAKE_PROPERTY(GeoCoordinates, float, longitude, setLongitude)

GeoCoordinates::GeoCoordinates(float latitude, float longitude) :
    d(*s_GeoCoordinates_shared_null())
{
    d.detach();
    d->latitude = latitude;
    d->longitude = longitude;
}

bool GeoCoordinates::isValid() const
{
    return !std::isnan(d->latitude) && !std::isnan(d->longitude);
}

// implemented manually, as NAN != NAN
bool GeoCoordinates::operator==(const GeoCoordinates &other) const
{
    if (!isValid() && !other.isValid()) {
        return true;
    }
    return qFuzzyCompare(d->latitude, other.d->latitude) && qFuzzyCompare(d->longitude, other.d->longitude);
}

class PostalAddressPrivate : public QSharedData
{
public:
    QString streetAddress;
    QString addressLocality;
    QString postalCode;
    QString addressRegion;
    QString addressCountry;
};

KITINERARY_MAKE_SIMPLE_CLASS(PostalAddress)
KITINERARY_MAKE_PROPERTY(PostalAddress, QString, streetAddress, setStreeAddress)
KITINERARY_MAKE_PROPERTY(PostalAddress, QString, addressLocality, setAddressLocality)
KITINERARY_MAKE_PROPERTY(PostalAddress, QString, postalCode, setPostalCode)
KITINERARY_MAKE_PROPERTY(PostalAddress, QString, addressRegion, setAddressRegion)
KITINERARY_MAKE_PROPERTY(PostalAddress, QString, addressCountry, setAddressCountry)
KITINERARY_MAKE_OPERATOR(PostalAddress)

bool PostalAddress::isEmpty() const
{
    return d->streetAddress.isEmpty() && d->addressLocality.isEmpty()
        && d->postalCode.isEmpty() && d->addressRegion.isEmpty()
        && d->addressCountry.isEmpty();
}


class PlacePrivate : public QSharedData
{
    KITINERARY_PRIVATE_BASE_GADGET(Place)
public:
    QString name;
    PostalAddress address;
    GeoCoordinates geo;
    QString telephone;
    QString identifier;
};

KITINERARY_MAKE_BASE_CLASS(Place)
KITINERARY_MAKE_PROPERTY(Place, QString, name, setName)
KITINERARY_MAKE_PROPERTY(Place, PostalAddress, address, setAddress)
KITINERARY_MAKE_PROPERTY(Place, GeoCoordinates, geo, setGeo)
KITINERARY_MAKE_PROPERTY(Place, QString, telephone, setTelephone)
KITINERARY_MAKE_PROPERTY(Place, QString, identifier, setIdentifier)
KITINERARY_MAKE_OPERATOR(Place)


class AirportPrivate : public PlacePrivate
{
    KITINERARY_PRIVATE_GADGET(Airport)
public:
    QString iataCode;
};

KITINERARY_MAKE_SUB_CLASS(Airport, Place)
KITINERARY_MAKE_PROPERTY(Airport, QString, iataCode, setIataCode)
KITINERARY_MAKE_OPERATOR(Airport)

class TrainStationPrivate : public PlacePrivate
{
    KITINERARY_PRIVATE_GADGET(TrainStation)
};
KITINERARY_MAKE_SUB_CLASS(TrainStation, Place)
KITINERARY_MAKE_OPERATOR(TrainStation)

class BusStationPrivate : public PlacePrivate
{
    KITINERARY_PRIVATE_GADGET(BusStation)
};
KITINERARY_MAKE_SUB_CLASS(BusStation, Place)
KITINERARY_MAKE_OPERATOR(BusStation)

class TouristAttractionPrivate: public PlacePrivate
{
    KITINERARY_PRIVATE_GADGET(TouristAttraction)
};
KITINERARY_MAKE_SUB_CLASS(TouristAttraction, Place)
KITINERARY_MAKE_OPERATOR(TouristAttraction)

}

template <>
KItinerary::PlacePrivate *QExplicitlySharedDataPointer<KItinerary::PlacePrivate>::clone()
{
    return d->clone();
}

#include "moc_place.cpp"
