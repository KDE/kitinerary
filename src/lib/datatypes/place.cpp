/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "place.h"
#include "datatypes_p.h"
#include "locationutil.h"

#include <cmath>

using namespace KItinerary;

namespace KItinerary {

class GeoCoordinatesPrivate : public QSharedData
{
public:
    float latitude = NAN;
    float longitude = NAN;
};

KITINERARY_MAKE_CLASS(GeoCoordinates)
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

KITINERARY_MAKE_CLASS(PostalAddress)
KITINERARY_MAKE_PROPERTY(PostalAddress, QString, streetAddress, setStreetAddress)
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

KITINERARY_MAKE_CLASS(Place)
KITINERARY_MAKE_PROPERTY(Place, QString, name, setName)
KITINERARY_MAKE_PROPERTY(Place, PostalAddress, address, setAddress)
KITINERARY_MAKE_PROPERTY(Place, GeoCoordinates, geo, setGeo)
KITINERARY_MAKE_PROPERTY(Place, QString, telephone, setTelephone)
KITINERARY_MAKE_PROPERTY(Place, QString, identifier, setIdentifier)
KITINERARY_MAKE_OPERATOR(Place)

QUrl Place::geoUri() const
{
    return LocationUtil::geoUri(*this);
}


class AirportPrivate : public PlacePrivate
{
    KITINERARY_PRIVATE_GADGET(Airport)
public:
    QString iataCode;
};

KITINERARY_MAKE_DERIVED_CLASS(Airport, Place)
KITINERARY_MAKE_PROPERTY(Airport, QString, iataCode, setIataCode)
KITINERARY_MAKE_OPERATOR(Airport)

class BoatTerminalPrivate : public PlacePrivate
{
    KITINERARY_PRIVATE_GADGET(BoatTerminal)
};
KITINERARY_MAKE_DERIVED_CLASS(BoatTerminal, Place)
KITINERARY_MAKE_OPERATOR(BoatTerminal)

class TrainStationPrivate : public PlacePrivate
{
    KITINERARY_PRIVATE_GADGET(TrainStation)
};
KITINERARY_MAKE_DERIVED_CLASS(TrainStation, Place)
KITINERARY_MAKE_OPERATOR(TrainStation)

class BusStationPrivate : public PlacePrivate
{
    KITINERARY_PRIVATE_GADGET(BusStation)
};
KITINERARY_MAKE_DERIVED_CLASS(BusStation, Place)
KITINERARY_MAKE_OPERATOR(BusStation)

class TouristAttractionPrivate: public PlacePrivate
{
    KITINERARY_PRIVATE_GADGET(TouristAttraction)
};
KITINERARY_MAKE_DERIVED_CLASS(TouristAttraction, Place)
KITINERARY_MAKE_OPERATOR(TouristAttraction)

}

template <>
KItinerary::PlacePrivate *QExplicitlySharedDataPointer<KItinerary::PlacePrivate>::clone()
{
    return d->clone();
}

#include "moc_place.cpp"
