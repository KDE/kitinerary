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
    var bookingRef = text.match(/[Bb]ooking reference: ([A-Z0-9]{6})/);

    var pos = 0;
    while (true) {
        var flightLine = text.substr(pos).match(/Flight:\s+([A-Z0-9]{2}) *([0-9]{3,4}), ([A-Za-z0-9 ]*)\n/);
        if (!flightLine)
            break;
        var index = flightLine.index + flightLine[0].length;

        var res = JsonLd.newObject("FlightReservation");
        res.reservationNumber = bookingRef[1];
        res.reservationFor = JsonLd.newObject("Flight");
        res.reservationFor.flightNumber = flightLine[2];
        res.reservationFor.airline = JsonLd.newObject("Airline");
        res.reservationFor.airline.iataCode = flightLine[1];
        res.reservationFor.airline.name = flightLine[3];

        var depDate = text.substr(pos + index).match(/Date: *[A-Z][a-z]{2} (.*)\n/);
        if (depDate)
            index += depDate.index + depDate[0].length;
        var depLine = text.substr(pos + index).match(/Departure: *([0-9]+:[0-9]+) *(.*) *\(([A-Z]{3})\)/);
        if (!depLine)
            break;
        index += depLine.index + depLine[0].length;
        res.reservationFor.departureTime = JsonLd.toDateTime(depDate[1] + " " + depLine[1], "d MMM yyyy hh:mm", "en");
        res.reservationFor.departureAirport = JsonLd.newObject("Airport");
        res.reservationFor.departureAirport.name = depLine[2];
        res.reservationFor.departureAirport.iataCode = depLine[3];

        var arrDate = text.substr(pos + index).match(/Date: *[A-Z][a-z]{2} (.*)\n/);
        if (arrDate)
            index += arrDate.index + arrDate[0].length;
        var arrLine = text.substr(pos + index).match(/Arrival: *([0-9]+:[0-9]+) *(.*) *\(([A-Z]{3})\)/);
        if (!arrLine)
            break;
        index += arrLine.index + arrLine[0].length;
        res.reservationFor.arrivalTime = JsonLd.toDateTime(arrDate[1] + " " + arrLine[1], "d MMM yyyy hh:mm", "en");
        res.reservationFor.arrivalAirport = JsonLd.newObject("Airport");
        res.reservationFor.arrivalAirport.name = arrLine[2];
        res.reservationFor.arrivalAirport.iataCode = arrLine[3];

        reservations.push(res);
        if (index == 0)
            break;
        pos += index;
    }

    return reservations;
}
