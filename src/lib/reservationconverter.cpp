/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "reservationconverter.h"
#include "json/jsonldfilterengine.h"

#include <QJsonObject>

using namespace Qt::Literals::StringLiterals;
using namespace KItinerary;

static constexpr const JsonLdFilterEngine::TypeMapping train_to_bus_type_map[] = {
    { "TrainReservation", "BusReservation" },
    { "TrainStation", "BusStation" },
    { "TrainTrip", "BusTrip" },
};

static constexpr const JsonLdFilterEngine::PropertyMapping train_to_bus_property_map[] = {
    { "BusTrip", "arrivalStation", "arrivalBusStop" },
    { "BusTrip", "departureStation", "departureBusStop" },
    { "BusTrip", "trainName", "busName" },
    { "BusTrip", "trainNumber", "busNumber" },
};

QJsonObject ReservationConverter::trainToBus(const QJsonObject &trainRes)
{
    QJsonObject obj = trainRes;
    JsonLdFilterEngine filterEngine;
    filterEngine.setTypeMappings(train_to_bus_type_map);
    filterEngine.setPropertyMappings(train_to_bus_property_map);
    filterEngine.filterRecursive(obj);
    return obj;
}

static constexpr const JsonLdFilterEngine::TypeMapping bus_to_train_type_map[] = {
    { "BusReservation", "TrainReservation" },
    { "BusStation", "TrainStation" },
    { "BusStop", "TrainStation" },
    { "BusTrip", "TrainTrip" },
};

static constexpr const JsonLdFilterEngine::PropertyMapping bus_to_train_property_map[] = {
    { "TrainTrip", "arrivalBusStop", "arrivalStation" },
    { "TrainTrip", "departureBusStop", "departureStation" },
    { "TrainTrip", "busCompany", "provider" },
    { "TrainTrip", "busName", "trainName" },
    { "TrainTrip", "busNumber", "trainNumber" },
};

QJsonObject ReservationConverter::busToTrain(const QJsonObject &busRes)
{
    QJsonObject obj = busRes;
    JsonLdFilterEngine filterEngine;
    filterEngine.setTypeMappings(bus_to_train_type_map);
    filterEngine.setPropertyMappings(bus_to_train_property_map);
    filterEngine.filterRecursive(obj);
    return obj;
}

static constexpr const JsonLdFilterEngine::TypeMapping flight_to_train_type_map[] = {
    { "Airline", "Organization" },
    { "Airport", "TrainStation" },
    { "Flight", "TrainTrip" },
    { "FlightReservation", "TrainReservation" },
};

static constexpr const JsonLdFilterEngine::PropertyMapping flight_to_train_property_map[] = {
    { "Organization", "iataCode", "identifier" },
    { "TrainStation", "iataCode", "identifier" },
    { "TrainTrip", "airline", "provider" },
    { "TrainTrip", "arrivalAirport", "arrivalStation" },
    { "TrainTrip", "departureAirport", "departureStation" },
};

static void addIataIdentifierPrefix(QJsonObject &obj, QLatin1StringView key)
{
    auto o = obj.value(key).toObject();
    if (const auto id = o.value("identifier"_L1).toString(); !id.isEmpty()) {
        o["identifier"_L1] = QString("iata:"_L1 + id);
        obj[key] = o;
    }
}

QJsonObject ReservationConverter::flightToTrain(const QJsonObject &flightRes)
{
    QJsonObject obj = flightRes;
    JsonLdFilterEngine filterEngine;
    filterEngine.setTypeMappings(flight_to_train_type_map);
    filterEngine.setPropertyMappings(flight_to_train_property_map);
    filterEngine.filterRecursive(obj);

    // changes that also need proeprty value adjustments
    auto flight = obj.value("reservationFor"_L1).toObject();
    addIataIdentifierPrefix(flight, "provider"_L1);
    addIataIdentifierPrefix(flight, "departureStation"_L1);
    addIataIdentifierPrefix(flight, "arrivalStation"_L1);
    obj["reservationFor"_L1] = flight;

    return obj;
}
