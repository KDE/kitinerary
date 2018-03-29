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
//     if (pass.transitType != KPkPass.BoardinPass.Air) { // TODO this needs to be registered in the engine
//         return null;
//     }

    var res = JsonLd.newObject("FlightReservation");
    res.ticketToken = pass.barcodes[0].message; // TODO this loses the barcode format and produces the wrong output format
    res.airplaneSeat = pass.field["seat"].value;
    res.reservationFor = JsonLd.newObject("Flight");
    res.reservationFor.departureGate = pass.field["gate"].value;
    res.reservationFor.departureAirport = JsonLd.newObject("Airport");
    res.reservationFor.departureAirport.iataCode = pass.field["origin"].value;
    res.reservationFor.departureAirport.name = pass.field["origin"].label;
    res.reservationFor.departureAirport.geo = JsonLd.newObject("GeoCoordinates");
    res.reservationFor.departureAirport.geo.latitude = pass.locations[0].latitude;
    res.reservationFor.departureAirport.geo.longitude = pass.locations[0].longitude;
    res.reservationFor.arrivalAirport = JsonLd.newObject("Airport");
    res.reservationFor.arrivalAirport.iataCode = pass.field["destination"].value;
    res.reservationFor.arrivalAirport.name = pass.field["destination"].label;
    res.reservationNumber = pass.field["filekey"].value;
    res.reservationFor.boardingTime = pass.relevantDate;
    // TODO flight number read from "flight" field (value is "LHxxxx")

    return res;
}
