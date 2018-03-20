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

/** Abstract base class for reservations.
 *  @see https://schema.org/Reservation
 */
class KITINERARY_EXPORT Reservation
{
    KITINERARY_ABSTRACT_GADGET(Reservation)
    KITINERARY_PROPERTY(QString, reservationNumber, setReservationNumber)
    KITINERARY_PROPERTY(QVariant, reservationFor, setReservationFor)
    KITINERARY_PROPERTY(QVariant, reservedTicket, setReservedTicket)

    // Google extension
    KITINERARY_PROPERTY(QUrl, cancelReservationUrl, setCancelReservationUrl)
    KITINERARY_PROPERTY(QUrl, modifyReservationUrl, setModifyReservationUrl)
    KITINERARY_PROPERTY(QString, ticketToken, setTicketToken)
    KITINERARY_PROPERTY(QUrl, url, setUrl)
protected:
    detail::shared_data_ptr<ReservationPrivate> d;
};

/** A hotel reservation.
 *  @see https://schema.org/LodgingReservation
 */
class KITINERARY_EXPORT LodgingReservation : public Reservation
{
    KITINERARY_GADGET(LodgingReservation)
    KITINERARY_PROPERTY(QDateTime, checkinDate, setCheckinDate)
    KITINERARY_PROPERTY(QDateTime, checkoutDate, setCheckoutDate)

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
class KITINERARY_EXPORT BusReservation : protected Reservation
{
    KITINERARY_GADGET(BusReservation)
};

}

Q_DECLARE_METATYPE(KItinerary::FlightReservation)
Q_DECLARE_METATYPE(KItinerary::LodgingReservation)
Q_DECLARE_METATYPE(KItinerary::TrainReservation)
Q_DECLARE_METATYPE(KItinerary::BusReservation)

#endif // KITINERARY_RESERVATION_H

