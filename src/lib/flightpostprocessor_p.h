/*
   SPDX-FileCopyrightText: 2017-2019 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_FLIGHTPOSTPROCESSOR_P_H
#define KITINERARY_FLIGHTPOSTPROCESSOR_P_H

#include "knowledgedb/iatacode.h"

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
    void pickAirportByDistance(int duration, const std::vector<KnowledgeDb::IataCode> &startCodes, std::vector<KnowledgeDb::IataCode> &codes) const;

    std::vector<KnowledgeDb::IataCode> m_departureCodes;
    std::vector<KnowledgeDb::IataCode> m_arrivalCodes;
};

}

#endif // KITINERARY_FLIGHTPOSTPROCESSOR_P_H
