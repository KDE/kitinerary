/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kitinerary_export.h"
#include "datatypes.h"
#include "organization.h"
#include "place.h"
#include "programmembership.h"

class QUrl;

namespace KItinerary {

class ReservationPrivate;

/** Abstract base class for reservations.
 *  @see https://schema.org/Reservation
 */
class KITINERARY_EXPORT Reservation
{
public:
    /** Reservation status enum.
     *  @see https://schema.org/ReservationStatusType
     */
    enum ReservationStatus {
        ReservationConfirmed,
        ReservationCancelled,
        ReservationHold,
        ReservationPending,
    };
    Q_ENUM(ReservationStatus)

    KITINERARY_GADGET(Reservation)
    KITINERARY_PROPERTY(QString, reservationNumber, setReservationNumber)
    KITINERARY_PROPERTY(QVariant, reservationFor, setReservationFor)
    KITINERARY_PROPERTY(QVariant, reservedTicket, setReservedTicket)
    KITINERARY_PROPERTY(QVariant, underName, setUnderName)
    KITINERARY_PROPERTY(KItinerary::Organization, provider, setProvider)
    KITINERARY_PROPERTY(QUrl, url, setUrl)
    KITINERARY_PROPERTY(QVariantList, potentialAction, setPotentialAction)
    KITINERARY_PROPERTY(QDateTime, modifiedTime, setModifiedTime)
    KITINERARY_PROPERTY(QVariantList, subjectOf, setSubjectOf)
    KITINERARY_PROPERTY(ReservationStatus, reservationStatus, setReservationStatus)
    KITINERARY_PROPERTY(KItinerary::ProgramMembership, programMembershipUsed, setProgramMembershipUsed)

    // KDE extensions
    /** Pass type identifier of an associated Apple Wallet boarding pass.
     *  @see KPkPass::Pass::passTypeIdentifier
     */
    KITINERARY_PROPERTY(QString, pkpassPassTypeIdentifier, setPkpassPassTypeIdentifier)
    /** Serial number of an associated Apple Wallet boarding pass.
     *  @see KPkPass::Pass::serialNumber
     */
    KITINERARY_PROPERTY(QString, pkpassSerialNumber, setPkpassSerialNumber)
protected:
    ///@cond internal
    QExplicitlySharedDataPointer<ReservationPrivate> d;
    ///@endcond
};

class LodgingReservationPrivate;

/** A hotel reservation.
 *  @see https://schema.org/LodgingReservation
 *  @see https://developers.google.com/gmail/markup/reference/hotel-reservation
 */
class KITINERARY_EXPORT LodgingReservation : public Reservation
{
    KITINERARY_GADGET(LodgingReservation)
    KITINERARY_PROPERTY(QDateTime, checkinTime, setCheckinTime)
    KITINERARY_PROPERTY(QDateTime, checkoutTime, setCheckoutTime)
};

class FlightReservationPrivate;

/** A flight reservation.
 *  @see https://schema.org/FlightReservation
 *  @see https://developers.google.com/gmail/markup/reference/flight-reservation
 */
class KITINERARY_EXPORT FlightReservation : public Reservation
{
    KITINERARY_GADGET(FlightReservation)
    /** Passenger sequence number
     *  Despite the name, do not expect this to be a number, infants without
     *  their own seat get vendor-defined codes here for example.
     *  @see https://schema.org/passengerSequenceNumber
     */
    KITINERARY_PROPERTY(QString, passengerSequenceNumber, setPassengerSequenceNumber)

    // Google extensions
    KITINERARY_PROPERTY(QString, airplaneSeat, setAirplaneSeat)
    KITINERARY_PROPERTY(QString, boardingGroup, setBoardingGroup)
};

class TrainReservationPrivate;

/** A train reservation.
 *  @see https://schema.org/TrainReservation
 */
class KITINERARY_EXPORT TrainReservation : public Reservation
{
    KITINERARY_GADGET(TrainReservation)
};

class BusReservationPrivate;

/** A bus reservation.
 *  @see https://schema.org/BusReservation
 */
class KITINERARY_EXPORT BusReservation : public Reservation
{
    KITINERARY_GADGET(BusReservation)
};

class FoodEstablishmentReservationPrivate;

/** A restaurant reservation.
 *  @see https://schema.org/FoodEstablishmentReservation
 *  @see https://developers.google.com/gmail/markup/reference/restaurant-reservation
 */
class KITINERARY_EXPORT FoodEstablishmentReservation : public Reservation
{
    KITINERARY_GADGET(FoodEstablishmentReservation)
    KITINERARY_PROPERTY(QDateTime, endTime, setEndTime)
    KITINERARY_PROPERTY(int, partySize, setPartySize)
    KITINERARY_PROPERTY(QDateTime, startTime, setStartTime)
};

class EventReservationPrivate;

/** An event reservation.
 *  @see https://schema.org/EventReservation
 *  @see https://developers.google.com/gmail/markup/reference/event-reservation
 */
class KITINERARY_EXPORT EventReservation : public Reservation
{
    KITINERARY_GADGET(EventReservation)
};

class RentalCarReservationPrivate;

/** A Rental Car reservation.
 *  @see https://developers.google.com/gmail/markup/reference/rental-car
 */
class KITINERARY_EXPORT RentalCarReservation : public Reservation
{
    KITINERARY_GADGET(RentalCarReservation)
    KITINERARY_PROPERTY(QDateTime, dropoffTime, setDropoffTime)
    KITINERARY_PROPERTY(QDateTime, pickupTime, setPickupTime)
    KITINERARY_PROPERTY(KItinerary::Place, pickupLocation, setPickupLocation)
    KITINERARY_PROPERTY(KItinerary::Place, dropoffLocation, setDropoffLocation)
};

class TaxiReservationPrivate;

/** A Taxi reservation.
 *  @see https://schema.org/TaxiReservation
 */
class KITINERARY_EXPORT TaxiReservation : public Reservation
{
    KITINERARY_GADGET(TaxiReservation)
    KITINERARY_PROPERTY(QDateTime, pickupTime, setPickupTime)
    KITINERARY_PROPERTY(KItinerary::Place, pickupLocation, setPickupLocation)
};

class BoatReservationPrivate;

/** A boat or ferry reservation.
 *  @see https://schema.org/BoatReservation
 */
class KITINERARY_EXPORT BoatReservation : public Reservation
{
    KITINERARY_GADGET(BoatReservation)
};


}

Q_DECLARE_METATYPE(KItinerary::Reservation::ReservationStatus)
Q_DECLARE_METATYPE(KItinerary::FlightReservation)
Q_DECLARE_METATYPE(KItinerary::LodgingReservation)
Q_DECLARE_METATYPE(KItinerary::TrainReservation)
Q_DECLARE_METATYPE(KItinerary::BusReservation)
Q_DECLARE_METATYPE(KItinerary::FoodEstablishmentReservation)
Q_DECLARE_METATYPE(KItinerary::EventReservation)
Q_DECLARE_METATYPE(KItinerary::RentalCarReservation)
Q_DECLARE_METATYPE(KItinerary::TaxiReservation)
Q_DECLARE_METATYPE(KItinerary::BoatReservation)
