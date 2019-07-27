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
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "reservation.h"
#include "organization.h"
#include "datatypes_p.h"

#include <QDateTime>
#include <QUrl>
#include <QVariant>
#include <QVector>

using namespace KItinerary;

namespace KItinerary {

class ReservationPrivate : public QSharedData
{
    KITINERARY_PRIVATE_BASE_GADGET(Reservation)
public:
    QString reservationNumber;
    QVariant reservationFor;
    QVariant reservedTicket;
    QVariant underName;
    QUrl url;
    QString pkpassPassTypeIdentifier;
    QString pkpassSerialNumber;
    Organization provider;
    QVariantList potentialAction;
    QDateTime modifiedTime;
    QVariantList subjectOf;
};

KITINERARY_MAKE_BASE_CLASS(Reservation)
KITINERARY_MAKE_PROPERTY(Reservation, QString, reservationNumber, setReservationNumber)
KITINERARY_MAKE_PROPERTY(Reservation, QVariant, reservationFor, setReservationFor)
KITINERARY_MAKE_PROPERTY(Reservation, QVariant, reservedTicket, setReservedTicket)
KITINERARY_MAKE_PROPERTY(Reservation, QVariant, underName, setUnderName)
KITINERARY_MAKE_PROPERTY(Reservation, QUrl, url, setUrl)
KITINERARY_MAKE_PROPERTY(Reservation, QString, pkpassPassTypeIdentifier, setPkpassPassTypeIdentifier)
KITINERARY_MAKE_PROPERTY(Reservation, QString, pkpassSerialNumber, setPkpassSerialNumber)
KITINERARY_MAKE_PROPERTY(Reservation, Organization, provider, setProvider)
KITINERARY_MAKE_PROPERTY(Reservation, QVariantList, potentialAction, setPotentialAction)
KITINERARY_MAKE_PROPERTY(Reservation, QDateTime, modifiedTime, setModifiedTime)
KITINERARY_MAKE_PROPERTY(Reservation, QVariantList, subjectOf, setSubjectOf)
KITINERARY_MAKE_OPERATOR(Reservation)

class LodgingReservationPrivate : public ReservationPrivate
{
    KITINERARY_PRIVATE_GADGET(LodgingReservation)
public:
    QDateTime checkinTime;
    QDateTime checkoutTime;
};
KITINERARY_MAKE_SUB_CLASS(LodgingReservation, Reservation)
KITINERARY_MAKE_PROPERTY(LodgingReservation, QDateTime, checkinTime, setCheckinTime)
KITINERARY_MAKE_PROPERTY(LodgingReservation, QDateTime, checkoutTime, setCheckoutTime)
KITINERARY_MAKE_OPERATOR(LodgingReservation)


class FlightReservationPrivate : public ReservationPrivate
{
    KITINERARY_PRIVATE_GADGET(FlightReservation)
public:
    QString passengerSequenceNumber;
    QString airplaneSeat;
    QString boardingGroup;
};

KITINERARY_MAKE_SUB_CLASS(FlightReservation, Reservation)
KITINERARY_MAKE_PROPERTY(FlightReservation, QString, passengerSequenceNumber, setPassengerSequenceNumber)
KITINERARY_MAKE_PROPERTY(FlightReservation, QString, airplaneSeat, setAirplaneSeat)
KITINERARY_MAKE_PROPERTY(FlightReservation, QString, boardingGroup, setBoardingGroup)
KITINERARY_MAKE_OPERATOR(FlightReservation)

class TrainReservationPrivate : public ReservationPrivate
{
    KITINERARY_PRIVATE_GADGET(TrainReservation)
};
KITINERARY_MAKE_SUB_CLASS(TrainReservation, Reservation)
KITINERARY_MAKE_OPERATOR(TrainReservation)

class BusReservationPrivate : public ReservationPrivate
{
    KITINERARY_PRIVATE_GADGET(BusReservation)
};
KITINERARY_MAKE_SUB_CLASS(BusReservation, Reservation)
KITINERARY_MAKE_OPERATOR(BusReservation)

class FoodEstablishmentReservationPrivate : public ReservationPrivate
{
    KITINERARY_PRIVATE_GADGET(FoodEstablishmentReservation)
public:
    QDateTime endTime;
    QDateTime startTime;
    int partySize = 0;
};
KITINERARY_MAKE_SUB_CLASS(FoodEstablishmentReservation, Reservation)
KITINERARY_MAKE_PROPERTY(FoodEstablishmentReservation, QDateTime, endTime, setEndTime)
KITINERARY_MAKE_PROPERTY(FoodEstablishmentReservation, int, partySize, setPartySize)
KITINERARY_MAKE_PROPERTY(FoodEstablishmentReservation, QDateTime, startTime, setStartTime)
KITINERARY_MAKE_OPERATOR(FoodEstablishmentReservation)

class EventReservationPrivate : public ReservationPrivate
{
    KITINERARY_PRIVATE_GADGET(EventReservation)
};
KITINERARY_MAKE_SUB_CLASS(EventReservation, Reservation)
KITINERARY_MAKE_OPERATOR(EventReservation)

class RentalCarReservationPrivate : public ReservationPrivate
{
    KITINERARY_PRIVATE_GADGET(RentalCarReservation)
public:
    QDateTime dropoffTime;
    QDateTime pickupTime;
    Place pickupLocation;
    Place dropoffLocation;
};
KITINERARY_MAKE_SUB_CLASS(RentalCarReservation, Reservation)
KITINERARY_MAKE_PROPERTY(RentalCarReservation, QDateTime, dropoffTime, setDropoffTime)
KITINERARY_MAKE_PROPERTY(RentalCarReservation, QDateTime, pickupTime, setPickupTime)
KITINERARY_MAKE_PROPERTY(RentalCarReservation, Place, pickupLocation, setPickupLocation)
KITINERARY_MAKE_PROPERTY(RentalCarReservation, Place, dropoffLocation, setDropoffLocation)
KITINERARY_MAKE_OPERATOR(RentalCarReservation)


class TaxiReservationPrivate : public ReservationPrivate
{
    KITINERARY_PRIVATE_GADGET(TaxiReservation)
public:
    QDateTime pickupTime;
    Place pickupLocation;
};
KITINERARY_MAKE_SUB_CLASS(TaxiReservation, Reservation)
KITINERARY_MAKE_PROPERTY(TaxiReservation, QDateTime, pickupTime, setPickupTime)
KITINERARY_MAKE_PROPERTY(TaxiReservation, Place, pickupLocation, setPickupLocation)
KITINERARY_MAKE_OPERATOR(TaxiReservation)


}

template <>
KItinerary::ReservationPrivate *QExplicitlySharedDataPointer<KItinerary::ReservationPrivate>::clone()
{
    return d->clone();
}

#include "moc_reservation.cpp"
