/*
   Copyright (c) 2017 Volker Krause <vkrause@kde.org>

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   This library is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
   License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.
*/

#ifndef DATATYPES_H
#define DATATYPES_H

#include <QDateTime>
#include <QString>
#include <QVariant>

#define SEMANTIC_GADGET \
    Q_GADGET \
    Q_PROPERTY(QString className READ className STORED false CONSTANT) \
    inline QString className() const { return QString::fromUtf8(staticMetaObject.className()); }

#define SEMANTIC_PROPERTY(Type, Name) \
    Q_PROPERTY(Type Name MEMBER m_##Name) \
    Type m_##Name;

/** @file
 *  The classes in here could possibly be auto-generated from the ontology defined by http://schema.org...
 */

class GeoCoordinates
{
    SEMANTIC_GADGET
    SEMANTIC_PROPERTY(float, latitude)
    SEMANTIC_PROPERTY(float, longitude)
public:
    GeoCoordinates();
    bool isValid() const;
};

class PostalAddress
{
    SEMANTIC_GADGET
    SEMANTIC_PROPERTY(QString, streetAddress)
    SEMANTIC_PROPERTY(QString, addressLocality)
    SEMANTIC_PROPERTY(QString, postalCode)
    SEMANTIC_PROPERTY(QString, addressCountry)
};

class Place
{
    Q_GADGET
    SEMANTIC_PROPERTY(QVariant, address)
    SEMANTIC_PROPERTY(QVariant, geo)
public:
    bool operator!=(const Place &other) const;
};

class Airport : protected Place
{
    SEMANTIC_GADGET
    SEMANTIC_PROPERTY(QString, name)
    SEMANTIC_PROPERTY(QString, iataCode)
public:
    bool operator!=(const Airport &other) const;
};

class Airline {
    SEMANTIC_GADGET
    SEMANTIC_PROPERTY(QString, name)
    SEMANTIC_PROPERTY(QString, iataCode)
public:
    bool operator!=(const Airline &other) const;
};

class TrainStation : protected Place
{
    SEMANTIC_GADGET
    SEMANTIC_PROPERTY(QString, name)
public:
    bool operator!=(const TrainStation &other) const;
};

class Flight
{
    SEMANTIC_GADGET
    SEMANTIC_PROPERTY(QString, flightNumber)
    SEMANTIC_PROPERTY(Airline, airline)
    SEMANTIC_PROPERTY(Airport, departureAirport)
    SEMANTIC_PROPERTY(QDateTime, departureTime)
    SEMANTIC_PROPERTY(Airport, arrivalAirport)
    SEMANTIC_PROPERTY(QDateTime, arrivalTime)

    Q_PROPERTY(QString departureTimeLocalized READ departureTimeLocalized STORED false CONSTANT)
    Q_PROPERTY(QString arrivalTimeLocalized READ arrivalTimeLocalized STORED false CONSTANT)
private:
    QString departureTimeLocalized() const;
    QString arrivalTimeLocalized() const;
};

class LodgingBusiness: protected Place
{
    SEMANTIC_GADGET
    SEMANTIC_PROPERTY(QString, name)
};

class TrainTrip
{
    SEMANTIC_GADGET
    SEMANTIC_PROPERTY(QString, arrivalPlatform)
    SEMANTIC_PROPERTY(TrainStation, arrivalStation)
    SEMANTIC_PROPERTY(QDateTime, arrivalTime)
    SEMANTIC_PROPERTY(QString, departurePlatform)
    SEMANTIC_PROPERTY(TrainStation, departureStation)
    SEMANTIC_PROPERTY(QDateTime, departureTime)
    SEMANTIC_PROPERTY(QString, trainName)
    SEMANTIC_PROPERTY(QString, trainNumber)

    Q_PROPERTY(QString departureTimeLocalized READ departureTimeLocalized STORED false CONSTANT)
    Q_PROPERTY(QString arrivalTimeLocalized READ arrivalTimeLocalized STORED false CONSTANT)

private:
    QString departureTimeLocalized() const;
    QString arrivalTimeLocalized() const;
};

class Reservation
{
    Q_GADGET
    SEMANTIC_PROPERTY(QString, reservationNumber)
    SEMANTIC_PROPERTY(QVariant, reservationFor)
};

class LodgingReservation : protected Reservation
{
    SEMANTIC_GADGET
    SEMANTIC_PROPERTY(QDateTime, checkinDate)
    SEMANTIC_PROPERTY(QDateTime, checkoutDate)

    Q_PROPERTY(QString checkinDateLocalized READ checkinDateLocalized STORED false CONSTANT)
    Q_PROPERTY(QString checkoutDateLocalized READ checkoutDateLocalized STORED false CONSTANT)
private:
    QString checkinDateLocalized() const;
    QString checkoutDateLocalized() const;
};

class FlightReservation : protected Reservation
{
    SEMANTIC_GADGET
};

class TrainReservation : protected Reservation
{
    SEMANTIC_GADGET
};

Q_DECLARE_METATYPE(GeoCoordinates)
Q_DECLARE_METATYPE(Airport)
Q_DECLARE_METATYPE(Airline)
Q_DECLARE_METATYPE(Flight)
Q_DECLARE_METATYPE(FlightReservation)
Q_DECLARE_METATYPE(LodgingBusiness)
Q_DECLARE_METATYPE(LodgingReservation)
Q_DECLARE_METATYPE(PostalAddress)
Q_DECLARE_METATYPE(TrainStation)
Q_DECLARE_METATYPE(TrainTrip)

#undef SEMANTIC_GADGET

#endif // DATATYPES_H
