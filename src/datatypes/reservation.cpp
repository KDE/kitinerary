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
#include "organization.h"
#include "datatypes_p.h"

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
    QUrl cancelReservationUrl;
    QUrl modifyReservationUrl;
    QString pkpassPassTypeIdentifier;
    QString pkpassSerialNumber;
    Organization provider;
};

KITINERARY_MAKE_BASE_CLASS(Reservation)
KITINERARY_MAKE_PROPERTY(Reservation, QString, reservationNumber, setReservationNumber)
KITINERARY_MAKE_PROPERTY(Reservation, QVariant, reservationFor, setReservationFor)
KITINERARY_MAKE_PROPERTY(Reservation, QVariant, reservedTicket, setReservedTicket)
KITINERARY_MAKE_PROPERTY(Reservation, QVariant, underName, setUnderName)
KITINERARY_MAKE_PROPERTY(Reservation, QUrl, cancelReservationUrl, setCancelReservationUrl)
KITINERARY_MAKE_PROPERTY(Reservation, QUrl, modifyReservationUrl, setModifyReservationUrl)
KITINERARY_MAKE_PROPERTY(Reservation, QUrl, url, setUrl)
KITINERARY_MAKE_PROPERTY(Reservation, QString, pkpassPassTypeIdentifier, setPkpassPassTypeIdentifier)
KITINERARY_MAKE_PROPERTY(Reservation, QString, pkpassSerialNumber, setPkpassSerialNumber)
KITINERARY_MAKE_PROPERTY(Reservation, Organization, provider, setProvider)

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

QString LodgingReservation::checkinDateLocalized() const
{
    K_D(const LodgingReservation);
    return QLocale().toString(d->checkinTime.date(), QLocale::ShortFormat);
}

QString LodgingReservation::checkoutDateLocalized() const
{
    K_D(const LodgingReservation);
    return QLocale().toString(d->checkoutTime.date(), QLocale::ShortFormat);
}


class FlightReservationPrivate : public ReservationPrivate
{
    KITINERARY_PRIVATE_GADGET(FlightReservation)
public:
    QString airplaneSeat;
    QString boardingGroup;
    QUrl ticketDownloadUrl;
};

KITINERARY_MAKE_SUB_CLASS(FlightReservation, Reservation)
KITINERARY_MAKE_PROPERTY(FlightReservation, QString, airplaneSeat, setAirplaneSeat)
KITINERARY_MAKE_PROPERTY(FlightReservation, QString, boardingGroup, setBoardingGroup)
KITINERARY_MAKE_PROPERTY(FlightReservation, QUrl, ticketDownloadUrl, setTicketDownloadUrl)

class TrainReservationPrivate : public ReservationPrivate
{
    KITINERARY_PRIVATE_GADGET(TrainReservation)
};
KITINERARY_MAKE_SUB_CLASS(TrainReservation, Reservation)

class BusReservationPrivate : public ReservationPrivate
{
    KITINERARY_PRIVATE_GADGET(BusReservation)
};
KITINERARY_MAKE_SUB_CLASS(BusReservation, Reservation)

}

template <>
KItinerary::ReservationPrivate *QExplicitlySharedDataPointer<KItinerary::ReservationPrivate>::clone()
{
    return d->clone();
}

#include "moc_reservation.cpp"
