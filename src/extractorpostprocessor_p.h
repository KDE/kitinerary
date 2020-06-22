/*
   Copyright (c) 2017-2019 Volker Krause <vkrause@kde.org>

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

#include "extractorvalidator.h"

#include <QDateTime>
#include <QVariant>
#include <QVector>

#ifndef KITINERARY_EXTRACTORPOSTPROCESSOR_P_H
#define KITINERARY_EXTRACTORPOSTPROCESSOR_P_H

namespace KItinerary {

class BusReservation;
class BusTrip;
class Event;
class EventReservation;
class Flight;
class FlightReservation;
class FoodEstablishmentReservation;
class GeoCoordinates;
class LodgingReservation;
class Person;
class PostalAddress;
class RentalCar;
class RentalCarReservation;
class TaxiReservation;
class TouristAttractionVisit;
class TrainReservation;
class TrainStation;
class TrainTrip;

class ExtractorPostprocessorPrivate
{
public:
    void mergeOrAppend(const QVariant &elem);

    QVariant processFlightReservation(FlightReservation res) const;

    TrainReservation processTrainReservation(TrainReservation res) const;
    TrainTrip processTrainTrip(TrainTrip trip) const;
    TrainStation processTrainStation(TrainStation station) const;
    QDateTime processTrainTripTime(QDateTime dt, const TrainStation &station) const;

    BusReservation processBusReservation(BusReservation res) const;
    BusTrip processBusTrip(BusTrip trip) const;

    LodgingReservation processLodgingReservation(LodgingReservation res) const;
    FoodEstablishmentReservation processFoodEstablishmentReservation(FoodEstablishmentReservation res) const;
    TouristAttractionVisit processTouristAttractionVisit(TouristAttractionVisit visit) const;
    EventReservation processEventReservation(EventReservation res) const;
    RentalCarReservation processRentalCarReservation(RentalCarReservation res) const;
    RentalCar processRentalCar(RentalCar car) const;
    TaxiReservation processTaxiReservation(TaxiReservation res) const;
    Event processEvent(Event event) const;

    template <typename T> T processReservation(T res) const;
    Person processPerson(Person person) const;
    template <typename T> static T processPlace(T place);
    static PostalAddress processAddress(PostalAddress addr, const QString &phoneNumber, const GeoCoordinates &geo);
    static QString processPhoneNumber(const QString &phoneNumber, const PostalAddress &addr);
    QVariantList processActions(QVariantList actions) const;
    template <typename T> QDateTime processTimeForLocation(QDateTime dt, const T &place) const;

    QVector<QVariant> m_data;
    QDateTime m_contextDate;
    ExtractorValidator m_validator;
    bool m_resultFinalized = false;
    bool m_validationEnabled = true;
};

template<typename T> inline T ExtractorPostprocessorPrivate::processPlace(T place)
{
    place.setName(place.name().simplified());
    auto addr = processAddress(place.address(), place.telephone(), place.geo());
    place.setAddress(addr);
    place.setTelephone(processPhoneNumber(place.telephone(), place.address()));
    return place;
}

}
#endif
