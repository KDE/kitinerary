/*
   SPDX-FileCopyrightText: 2017-2019 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "knowledgedb/iatacode.h"

#include <chrono>
#include <vector>

class QDateTime;

namespace KItinerary {

class Airline;
class Airport;
class Flight;

/** Post-processing logic for flight reservations. */
class FlightPostProcessor
{
public:
    Flight processFlight(Flight flight);

private:
    Airport processAirport(Airport airport, const std::vector<KnowledgeDb::IataCode> &codes) const;
    Airline processAirline(Airline airline) const;
    QDateTime processFlightTime(QDateTime dt, const Flight &flight, const std::vector<KnowledgeDb::IataCode> &codes) const;

    void lookupAirportCodes(const Airport &airport, std::vector<KnowledgeDb::IataCode> &codes) const;
    void pickAirportByDistance(std::chrono::seconds duration, const std::vector<KnowledgeDb::IataCode> &startCodes, std::vector<KnowledgeDb::IataCode> &codes) const;

    std::vector<KnowledgeDb::IataCode> m_departureCodes;
    std::vector<KnowledgeDb::IataCode> m_arrivalCodes;
};

}

