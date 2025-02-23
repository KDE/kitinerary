/*
   SPDX-FileCopyrightText: 2017-2019 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_EXTRACTORPOSTPROCESSOR_P_H
#define KITINERARY_EXTRACTORPOSTPROCESSOR_P_H

#include "stringutil.h"

#include <QDateTime>
#include <QList>
#include <QVariant>

namespace KItinerary {

class BoatReservation;
class BoatTrip;
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
class Seat;
class TaxiReservation;
class Ticket;
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
    /** Process train station or bus stop. */
    template <typename T>
    [[nodiscard]] static T processStation(T station);
    template <typename T>
    [[nodiscard]] QDateTime processTripTime(QDateTime dt, QDate departureDay, const T &place) const;

    BusReservation processBusReservation(BusReservation res) const;
    BusTrip processBusTrip(BusTrip trip) const;

    BoatReservation processBoatReservation(BoatReservation res) const;
    BoatTrip processBoatTrip(BoatTrip trip) const;

    LodgingReservation processLodgingReservation(LodgingReservation res) const;
    FoodEstablishmentReservation processFoodEstablishmentReservation(FoodEstablishmentReservation res) const;
    TouristAttractionVisit processTouristAttractionVisit(TouristAttractionVisit visit) const;
    EventReservation processEventReservation(EventReservation res) const;
    RentalCarReservation processRentalCarReservation(RentalCarReservation res) const;
    RentalCar processRentalCar(RentalCar car) const;
    TaxiReservation processTaxiReservation(TaxiReservation res) const;
    Event processEvent(Event event) const;

    Ticket processTicket(Ticket ticket) const;
    ProgramMembership processProgramMembership(ProgramMembership program) const;
    Seat processSeat(Seat seat) const;

    template <typename T> T processReservation(T res) const;
    Person processPerson(Person person) const;
    template <typename T> static T processPlace(T place);
    static PostalAddress processAddress(PostalAddress addr, const QString &phoneNumber, const GeoCoordinates &geo);
    static QString processPhoneNumber(const QString &phoneNumber, const PostalAddress &addr);
    QVariantList processActions(QVariantList actions) const;
    template <typename T> QDateTime processTimeForLocation(QDateTime dt, const T &place) const;

    QList<QVariant> m_data;
    QDateTime m_contextDate;
    bool m_resultFinalized = false;
};

template<typename T> inline T ExtractorPostprocessorPrivate::processPlace(T place)
{
    place.setName(StringUtil::clean(place.name()));
    auto addr = processAddress(place.address(), place.telephone(), place.geo());
    place.setAddress(addr);
    place.setTelephone(processPhoneNumber(place.telephone(), place.address()));
    return place;
}

}
#endif
