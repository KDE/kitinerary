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
    var bookingRef = text.match(/Individual reservation code:\s*\*\* ([A-Z0-9]{6})/);

    var pos = 0;
    while (true) {
        var flightLine = text.substr(pos).match(/Flight: ([0-9]{2}\.[0-9]{2}\.[0-9]{4})\s*\|\s*([A-Z0-9]{2}) ([0-9]{3,4}).*\n/);
        if (!flightLine)
            break;
        var idx = flightLine.index + flightLine[0].length;

        var res = JsonLd.newObject("FlightReservation");
        res.reservationNumber = bookingRef[1];
        res.reservationFor = JsonLd.newObject("Flight");
        res.reservationFor.flightNumber = flightLine[3];

        res.reservationFor.airline = JsonLd.newObject("Airline");
        res.reservationFor.airline.iataCode = flightLine[2];

        var opByLine = text.substr(pos + idx).match(/^\s*\* operated by (.*)\n/);
        if (opByLine) {
            idx += opByLine.index + opByLine[0].length;
            res.reservationFor.airline.name = opByLine[1];
        }

        var depLine = text.substr(pos + idx).match(/Departure:\s*([0-9]{2}:[0-9]{2})\s+(.*)\n/);
        if (!depLine)
            break;
        idx += depLine.index + depLine[0].length;
        res.reservationFor.departureTime = JsonLd.toDateTime(flightLine[1] + ' ' + depLine[1], "dd.MM.yyyy hh:mm", "en");
        res.reservationFor.departureAirport = JsonLd.newObject("Airport");
        res.reservationFor.departureAirport.name = depLine[2];

        var arrLine = text.substr(pos + idx).match(/Arrival:\s*([0-9]{2}:[0-9]{2})\s+(.*)\n/);
        if (!arrLine)
            break;
        idx += arrLine.index + arrLine[0].length;
        res.reservationFor.arrivalTime = JsonLd.toDateTime(flightLine[1] + ' ' + arrLine[1], "dd.MM.yyyy hh:mm", "en");
        res.reservationFor.arrivalAirport = JsonLd.newObject("Airport");
        res.reservationFor.arrivalAirport.name = arrLine[2];

        reservations.push(res);
        if (idx == 0)
            break;
        pos += idx;
    }

    return reservations;
}
