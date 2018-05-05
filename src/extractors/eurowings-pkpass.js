/*
   Copyright (c) 2018 Volker Krause <vkrause@kde.org>

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

function main(pass)
{
    var res = JsonLd.newObject("FlightReservation");
    res.reservationFor = JsonLd.newObject("Flight");
    res.reservationFor.departureGate = pass.field["gate"].value;
    res.reservationFor.departureAirport = JsonLd.newObject("Airport");
    res.reservationFor.departureAirport.name = pass.field["origin"].label;
    res.reservationFor.arrivalAirport = JsonLd.newObject("Airport");
    res.reservationFor.arrivalAirport.name = pass.field["destination"].label;
    res.reservationFor.boardingTime = JsonLd.toDateTime(pass.field["departureDate"].value + ' ' + pass.field["boarding"].value, "M/d/yyyy hh:mm", "en");

    res.reservationFor.airline = JsonLd.newObject("Airline");
    if (pass.field["operatingcarrier"])
        res.reservationFor.airline.name = pass.field["operatingcarrier"].value;

    return res;
}
