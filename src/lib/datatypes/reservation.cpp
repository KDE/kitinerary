/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "reservation.h"
#include "datatypes_p.h"

#include <QDateTime>
#include <QList>
#include <QUrl>
#include <QVariant>

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
    Reservation::ReservationStatus reservationStatus = Reservation::ReservationConfirmed;
    ProgramMembership programMembershipUsed;
};

KITINERARY_MAKE_CLASS(Reservation)
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
KITINERARY_MAKE_PROPERTY(Reservation, Reservation::ReservationStatus, reservationStatus, setReservationStatus)
KITINERARY_MAKE_PROPERTY(Reservation, ProgramMembership, programMembershipUsed, setProgramMembershipUsed)
KITINERARY_MAKE_OPERATOR(Reservation)

class LodgingReservationPrivate : public ReservationPrivate
{
    KITINERARY_PRIVATE_GADGET(LodgingReservation)
public:
    QDateTime checkinTime;
    QDateTime checkoutTime;
};
KITINERARY_MAKE_DERIVED_CLASS(LodgingReservation, Reservation)
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

KITINERARY_MAKE_DERIVED_CLASS(FlightReservation, Reservation)
KITINERARY_MAKE_PROPERTY(FlightReservation, QString, passengerSequenceNumber, setPassengerSequenceNumber)
KITINERARY_MAKE_PROPERTY(FlightReservation, QString, airplaneSeat, setAirplaneSeat)
KITINERARY_MAKE_PROPERTY(FlightReservation, QString, boardingGroup, setBoardingGroup)
KITINERARY_MAKE_OPERATOR(FlightReservation)

class TrainReservationPrivate : public ReservationPrivate
{
    KITINERARY_PRIVATE_GADGET(TrainReservation)
};
KITINERARY_MAKE_DERIVED_CLASS(TrainReservation, Reservation)
KITINERARY_MAKE_OPERATOR(TrainReservation)

class BusReservationPrivate : public ReservationPrivate
{
    KITINERARY_PRIVATE_GADGET(BusReservation)
};
KITINERARY_MAKE_DERIVED_CLASS(BusReservation, Reservation)
KITINERARY_MAKE_OPERATOR(BusReservation)

class FoodEstablishmentReservationPrivate : public ReservationPrivate
{
    KITINERARY_PRIVATE_GADGET(FoodEstablishmentReservation)
public:
    QDateTime endTime;
    QDateTime startTime;
    int partySize = 0;
};
KITINERARY_MAKE_DERIVED_CLASS(FoodEstablishmentReservation, Reservation)
KITINERARY_MAKE_PROPERTY(FoodEstablishmentReservation, QDateTime, endTime, setEndTime)
KITINERARY_MAKE_PROPERTY(FoodEstablishmentReservation, int, partySize, setPartySize)
KITINERARY_MAKE_PROPERTY(FoodEstablishmentReservation, QDateTime, startTime, setStartTime)
KITINERARY_MAKE_OPERATOR(FoodEstablishmentReservation)

class EventReservationPrivate : public ReservationPrivate
{
    KITINERARY_PRIVATE_GADGET(EventReservation)
};
KITINERARY_MAKE_DERIVED_CLASS(EventReservation, Reservation)
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
KITINERARY_MAKE_DERIVED_CLASS(RentalCarReservation, Reservation)
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
KITINERARY_MAKE_DERIVED_CLASS(TaxiReservation, Reservation)
KITINERARY_MAKE_PROPERTY(TaxiReservation, QDateTime, pickupTime, setPickupTime)
KITINERARY_MAKE_PROPERTY(TaxiReservation, Place, pickupLocation, setPickupLocation)
KITINERARY_MAKE_OPERATOR(TaxiReservation)


class BoatReservationPrivate : public ReservationPrivate
{
    KITINERARY_PRIVATE_GADGET(BoatReservation)
};
KITINERARY_MAKE_DERIVED_CLASS(BoatReservation, Reservation)
KITINERARY_MAKE_OPERATOR(BoatReservation)

}

template <>
KItinerary::ReservationPrivate *QExplicitlySharedDataPointer<KItinerary::ReservationPrivate>::clone()
{
    return d->clone();
}

#include "moc_reservation.cpp"
