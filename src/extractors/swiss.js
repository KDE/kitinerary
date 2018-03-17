/*
   Copyright (c) 2017 Volker Krause <vkrause@kde.org>

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

function main(text) {
    var reservations = new Array();
    var bookingRef = text.match(/Buchungsreferenz: ([A-Z0-9]{6})/);

    var pos = 0;
    while (true) {
        var flight = text.substr(pos).match(/Hinflug|Rückflug/);
        if (!flight)
            break;
        var index = flight.index + flight[0].length;

        var res = JsonLd.newObject("FlightReservation");
        res.reservationNumber = bookingRef[1];
        res.reservationFor = JsonLd.newObject("Flight");

        var leg = text.substr(pos + index).match(/  +(.+?) \(([A-Z]{3})\) - (.+?) \(([A-Z]{3})\) +[A-Z][a-z] (\d{2}.\d{2}.\d{4}) +(\d{2}:\d{2}) +[A-Z]{3} +(\d{2}:\d{2})  .*?  ([A-Z0-9]{2}) (\d{3,4})/);
        if (!leg)
            break;
        index += leg.index + leg[0].length;
        res.reservationFor.departureAirport = JsonLd.newObject("Airport");
        res.reservationFor.departureAirport.name = leg[1];
        res.reservationFor.departureAirport.iataCode = leg[2];
        res.reservationFor.arrivalAirport = JsonLd.newObject("Airport");
        res.reservationFor.arrivalAirport.name = leg[3];
        res.reservationFor.arrivalAirport.iataCode = leg[4];
        res.reservationFor.departureTime = JsonLd.toDateTime(leg[5] + ' ' + leg[6], "dd.MM.yyyy hh:mm", "en");
        res.reservationFor.arrivalTime = JsonLd.toDateTime(leg[5] + ' ' + leg[7], "dd.MM.yyyy hh:mm", "en");
        res.reservationFor.flightNumber = leg[9];
        res.reservationFor.airline = JsonLd.newObject("Airline");
        res.reservationFor.airline.iataCode = leg[8];
        // TODO: parse the operated by part to fill in airline name

        reservations.push(res);
        if (index == 0)
            break;
        pos += index;
    }

    return reservations;
}
