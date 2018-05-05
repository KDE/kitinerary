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
    var bookingRef = text.match(/Flug - Buchungscode\s+([0-9A-z]{6})/);
    if (!bookingRef)
        return reservations;

    var pos = bookingRef.index + bookingRef[0].length;
    while (true) {
        var departure = text.substr(pos).match(/[A-Z][a-z] (\d{1,2} \w{3} \d{2})\s+(\d{2}:\d{2})\s+(.+?)\n\t+\((.+?)\)\n/);
        if (!departure)
            break;
        var index = departure.index + departure[0].length;

        var res = JsonLd.newObject("FlightReservation");
        res.reservationNumber = bookingRef[1];
        res.reservationFor = JsonLd.newObject("Flight");
        res.reservationFor.departureTime = JsonLd.toDateTime(departure[1] + ' ' + departure[2], "d MMM yy hh:mm", "en");
        res.reservationFor.departureAirport = JsonLd.newObject("Airport");
        res.reservationFor.departureAirport.name = departure[3] + ", " + departure[4];

        var arrival = text.substr(pos + index).match(/[A-Z][a-z] (\d{1,2} \w{3} \d{2})\s+(\d{2}:\d{2})\s+(.+?)\n\s+\((.+?)\)\n/);
        if (!arrival)
            break;
        index += arrival.index + arrival[1].length;

        res.reservationFor.arrivalTime = JsonLd.toDateTime(arrival[1] + ' ' + arrival[2], "d MMM yy hh:mm", "en");
        res.reservationFor.arrivalAirport = JsonLd.newObject("Airport");
        res.reservationFor.arrivalAirport.name = arrival[3] + ", " + arrival[4];

        var flightNumber = text.substr(pos + index).match(/Flugnummer: ([A-Z]{2}) (\d{2,4})\n/);
        if (!flightNumber)
            break;
        index += flightNumber.index + flightNumber[0].length;
        res.reservationFor.flightNumber = flightNumber[2];
        res.reservationFor.airline = JsonLd.newObject("Airline");
        res.reservationFor.airline.iataCode = flightNumber[1];

        var opBy = text.substr(pos + index).match(/Durchgeführt von: (.*?)\n/);
        if (!opBy)
            break;
        index += opBy.index + opBy[0].length;
        res.reservationFor.airline.name = opBy[1];

        reservations.push(res);
        if (index == 0)
            break;
        pos += index;
    }
    return reservations;
}
