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

#ifndef KITINERARY_RESERVATION_H
#define KITINERARY_RESERVATION_H

#include "kitinerary_export.h"
#include "datatypes.h"

class QUrl;

namespace KItinerary {

class ReservationPrivate;

class Organization;

/** Abstract base class for reservations.
 *  @see https://schema.org/Reservation
 */
class KITINERARY_EXPORT Reservation
{
    KITINERARY_BASE_GADGET(Reservation)
    KITINERARY_PROPERTY(QString, reservationNumber, setReservationNumber)
    KITINERARY_PROPERTY(QVariant, reservationFor, setReservationFor)
    KITINERARY_PROPERTY(QVariant, reservedTicket, setReservedTicket)
    KITINERARY_PROPERTY(QVariant, underName, setUnderName)
    KITINERARY_PROPERTY(KItinerary::Organization, provider, setProvider)
    KITINERARY_PROPERTY(QUrl, url, setUrl)

    // Google extension
    KITINERARY_PROPERTY(QUrl, cancelReservationUrl, setCancelReservationUrl)
    KITINERARY_PROPERTY(QUrl, modifyReservationUrl, setModifyReservationUrl)

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

/** A hotel reservation.
 *  @see https://schema.org/LodgingReservation
 *  @see https://developers.google.com/gmail/markup/reference/hotel-reservation
 */
class KITINERARY_EXPORT LodgingReservation : public Reservation
{
    KITINERARY_GADGET(LodgingReservation)
    KITINERARY_PROPERTY(QDateTime, checkinTime, setCheckinTime)
    KITINERARY_PROPERTY(QDateTime, checkoutTime, setCheckoutTime)

    Q_PROPERTY(QString checkinDateLocalized READ checkinDateLocalized STORED false CONSTANT)
    Q_PROPERTY(QString checkoutDateLocalized READ checkoutDateLocalized STORED false CONSTANT)

private:
    QString checkinDateLocalized() const;
    QString checkoutDateLocalized() const;
};

/** A flight reservation.
 *  @see https://schema.org/FlightReservation
 *  @see https://developers.google.com/gmail/markup/reference/flight-reservation
 */
class KITINERARY_EXPORT FlightReservation : public Reservation
{
    KITINERARY_GADGET(FlightReservation)
    /** Passenger sequnce number
     *  Despite the name, do not expect this to be a number, infants without
     *  their own seat get vendor-defined codes here for example.
     *  @see https://schema.org/passengerSequenceNumber
     */
    KITINERARY_PROPERTY(QString, passengerSequenceNumber, setPassengerSequenceNumber)

    // Google extensions
    KITINERARY_PROPERTY(QString, airplaneSeat, setAirplaneSeat)
    KITINERARY_PROPERTY(QString, boardingGroup, setBoardingGroup)
    KITINERARY_PROPERTY(QUrl, ticketDownloadUrl, setTicketDownloadUrl)
};

/** A train reservation.
 *  @see https://schema.org/TrainReservation
 */
class KITINERARY_EXPORT TrainReservation : public Reservation
{
    KITINERARY_GADGET(TrainReservation)
};

/** A bus reservation.
 *  @see https://schema.org/BusReservation
 */
class KITINERARY_EXPORT BusReservation : public Reservation
{
    KITINERARY_GADGET(BusReservation)
};

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

}

Q_DECLARE_METATYPE(KItinerary::FlightReservation)
Q_DECLARE_METATYPE(KItinerary::LodgingReservation)
Q_DECLARE_METATYPE(KItinerary::TrainReservation)
Q_DECLARE_METATYPE(KItinerary::BusReservation)
Q_DECLARE_METATYPE(KItinerary::FoodEstablishmentReservation)

#endif // KITINERARY_RESERVATION_H

