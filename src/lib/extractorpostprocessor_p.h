/*
   SPDX-FileCopyrightText: 2017-2019 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
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
class ProgramMembership;
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
    QDateTime processTrainTripTime(QDateTime dt, QDate departureDay, const TrainStation &station) const;

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

    ProgramMembership processProgramMembership(ProgramMembership program) const;

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
