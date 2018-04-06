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

#include "mergeutil.h"

#include <KItinerary/Flight>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/Organization>
#include <KItinerary/Person>
#include <KItinerary/Reservation>

#include <QDate>
#include <QVariant>

using namespace KItinerary;

bool MergeUtil::isSameReservation(const QVariant& lhs, const QVariant& rhs)
{
    if (lhs.isNull() || rhs.isNull()) {
        return false;
    }
    if (lhs.userType() != rhs.userType()) {
        return false;
    }

    // flight: booking ref, flight number and departure day match
    if (lhs.userType() == qMetaTypeId<FlightReservation>()) {
        const auto lhsRes = lhs.value<FlightReservation>();
        const auto rhsRes = rhs.value<FlightReservation>();
        if (lhsRes.reservationNumber() != rhsRes.reservationNumber() || lhsRes.reservationNumber().isEmpty()) {
            return false;
        }
        const auto lhsFlight = lhsRes.reservationFor().value<Flight>();
        const auto rhsFlight = rhsRes.reservationFor().value<Flight>();
        if (!isSameFlight(lhsFlight, rhsFlight)) {
            return false;
        }
    }

    // TODO train/bus: booking ref, train number and depature day match
    // TODO hotel: booking ref, checkin day match

    // for all: underName either matches or is not set
    const auto lhsUN = JsonLdDocument::readProperty(lhs, "underName");
    const auto rhsUN = JsonLdDocument::readProperty(rhs, "underName");
    return lhsUN.isNull() || rhsUN.isNull() || isSamePerson(lhsUN.value<Person>(), rhsUN.value<Person>());
}

bool MergeUtil::isSameFlight(const Flight& lhs, const Flight& rhs)
{
    if (lhs.flightNumber().isEmpty() || rhs.flightNumber().isEmpty()) {
        return false;
    }

    return lhs.flightNumber() == rhs.flightNumber() && lhs.airline().iataCode() == rhs.airline().iataCode() && lhs.departureDay() == rhs.departureDay();
}

bool MergeUtil::isSamePerson(const Person& lhs, const Person& rhs)
{
    // TODO: extend this once Person has familiy and given name fields
    return lhs.name().compare(rhs.name(), Qt::CaseInsensitive) == 0;
}
