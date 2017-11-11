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
    Q_PROPERTY(QString className READ className CONSTANT) \
    inline QString className() const { return QString::fromUtf8(staticMetaObject.className()); }

#define SEMANTIC_PROPERTY(Type, Name) \
    Q_PROPERTY(Type Name MEMBER m_##Name) \
    Type m_##Name;

/** @file
 *  The classes in here could possibly be auto-generated from the ontology defined by http://schema.org...
 */

class Airport
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

class Flight
{
    SEMANTIC_GADGET
    SEMANTIC_PROPERTY(QString, flightNumber)
    SEMANTIC_PROPERTY(Airline, airline)
    SEMANTIC_PROPERTY(Airport, departureAirport)
    SEMANTIC_PROPERTY(QDateTime, departureTime)
    SEMANTIC_PROPERTY(Airport, arrivalAirport)
    SEMANTIC_PROPERTY(QDateTime, arrivalTime)

    Q_PROPERTY(QString departureTimeLocalized READ departureTimeLocalized CONSTANT)
    Q_PROPERTY(QString arrivalTimeLocalized READ arrivalTimeLocalized CONSTANT)
private:
    QString departureTimeLocalized() const;
    QString arrivalTimeLocalized() const;
};

class PostalAddress
{
    SEMANTIC_GADGET
    SEMANTIC_PROPERTY(QString, streetAddress)
    SEMANTIC_PROPERTY(QString, addressLocality)
    SEMANTIC_PROPERTY(QString, postalCode)
    SEMANTIC_PROPERTY(QString, addressCountry)
};

class LodgingBusiness
{
    SEMANTIC_GADGET
    SEMANTIC_PROPERTY(QString, name)
    SEMANTIC_PROPERTY(QVariant, address)
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

    Q_PROPERTY(QString checkinDateLocalized READ checkinDateLocalized CONSTANT)
    Q_PROPERTY(QString checkoutDateLocalized READ checkoutDateLocalized CONSTANT)
private:
    QString checkinDateLocalized() const;
    QString checkoutDateLocalized() const;
};

class FlightReservation : protected Reservation
{
    SEMANTIC_GADGET
};

Q_DECLARE_METATYPE(Airport)
Q_DECLARE_METATYPE(Airline)
Q_DECLARE_METATYPE(Flight)
Q_DECLARE_METATYPE(LodgingBusiness)
Q_DECLARE_METATYPE(LodgingReservation)
Q_DECLARE_METATYPE(FlightReservation)
Q_DECLARE_METATYPE(PostalAddress)

#undef SEMANTIC_GADGET

#endif // DATATYPES_H
