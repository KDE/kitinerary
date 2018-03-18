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

#include "reservation.h"
#include "datatypes_p.h"

#include <QUrl>
#include <QVariant>

using namespace KItinerary;

namespace KItinerary {

class ReservationPrivate : public detail::private_abstract_base<ReservationPrivate>
{
public:
    virtual ~ReservationPrivate() = default;
    QString reservationNumber;
    QVariant reservationFor;
    QVariant reservedTicket;
    QUrl cancelReservationUrl;
    QUrl modifyReservationUrl;
    QString ticketToken;
    QUrl url;
};

KITINERARY_MAKE_ABSTRACT_CLASS(Reservation)
KITINERARY_MAKE_PROPERTY(Reservation, QString, reservationNumber, setReservationNumber)
KITINERARY_MAKE_PROPERTY(Reservation, QVariant, reservationFor, setReservationFor)
KITINERARY_MAKE_PROPERTY(Reservation, QVariant, reservedTicket, setReservedTicket)
KITINERARY_MAKE_PROPERTY(Reservation, QUrl, cancelReservationUrl, setCancelReservationUrl)
KITINERARY_MAKE_PROPERTY(Reservation, QUrl, modifyReservationUrl, setModifyReservationUrl)
KITINERARY_MAKE_PROPERTY(Reservation, QString, ticketToken, setTicketToken)
KITINERARY_MAKE_PROPERTY(Reservation, QUrl, url, setUrl)

class LodgingReservationPrivate : public detail::private_derived_base<LodgingReservationPrivate, ReservationPrivate>
{
public:
    QDateTime checkinDate;
    QDateTime checkoutDate;
};
KITINERARY_MAKE_SUB_CLASS(LodgingReservation, Reservation)
KITINERARY_MAKE_PROPERTY(LodgingReservation, QDateTime, checkinDate, setCheckinDate)
KITINERARY_MAKE_PROPERTY(LodgingReservation, QDateTime, checkoutDate, setCheckoutDate)

QString LodgingReservation::checkinDateLocalized() const
{
    K_D(const LodgingReservation);
    return QLocale().toString(d->checkinDate.date(), QLocale::ShortFormat);
}

QString LodgingReservation::checkoutDateLocalized() const
{
    K_D(const LodgingReservation);
    return QLocale().toString(d->checkoutDate.date(), QLocale::ShortFormat);
}


class FlightReservationPrivate : public detail::private_derived_base<FlightReservationPrivate, ReservationPrivate>
{
public:
    QString airplaneSeat;
    QString boardingGroup;
    QUrl ticketDownloadUrl;
};

KITINERARY_MAKE_SUB_CLASS(FlightReservation, Reservation)
KITINERARY_MAKE_PROPERTY(FlightReservation, QString, airplaneSeat, setAirplaneSeat)
KITINERARY_MAKE_PROPERTY(FlightReservation, QString, boardingGroup, setBoardingGroup)
KITINERARY_MAKE_PROPERTY(FlightReservation, QUrl, ticketDownloadUrl, setTicketDownloadUrl)

class TrainReservationPrivate : public detail::private_derived_base<TrainReservationPrivate, ReservationPrivate> {};
KITINERARY_MAKE_SUB_CLASS(TrainReservation, Reservation)
class BusReservationPrivate : public detail::private_derived_base<BusReservationPrivate, ReservationPrivate> {};
KITINERARY_MAKE_SUB_CLASS(BusReservation, Reservation)

}

#include "moc_reservation.cpp"
