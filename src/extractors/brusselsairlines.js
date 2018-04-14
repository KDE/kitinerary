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
    var bookingRef = text.match(/Booking reference:\s+([A-Z0-9]{6})/);

    var pos = 0;
    while (true) { // departure/return blocks
        var header = text.substr(pos).match(/Departure|Return/);
        if (!header)
            break;
        pos += header.index + header[0].length;

        var lastAirport = "";
        while (true) { // legs
            var res = JsonLd.newObject("FlightReservation");
            res.reservationNumber = bookingRef[1];
            res.reservationFor = JsonLd.newObject("Flight");

            var depAirport = null;
            if (lastAirport !== "")
                depAirport = text.substr(pos).match("(\n[ \t]*){5}(" + lastAirport + ")");
            else
                depAirport = text.substr(pos).match(/([A-Z][\S ]*)\n/);
            if (!depAirport)
                break;
            var idx = depAirport.index + depAirport[0].length;
            res.reservationFor.departureAirport = JsonLd.newObject("Airport");
            res.reservationFor.departureAirport.name = lastAirport != "" ? lastAirport : depAirport[1].trim();

            var depTime = text.substr(pos + idx).match(/([0-9]{2} [A-Za-z]{3} [0-9]{4}),\s*([0-9]{2}:[0-9]{2})/);
            if (!depTime)
                break;
            idx += depTime.index + depTime[0].length;
            res.reservationFor.departureTime = JsonLd.toDateTime(depTime[1] + ' ' + depTime[2], "dd MMM yyyy hh:mm", "en");

            var arrAirport = text.substr(pos + idx).match(/([A-Z][\S ]*)\n/);
            if (!arrAirport)
                break;
            idx += arrAirport.index + arrAirport[0].length;
            res.reservationFor.arrivalAirport = JsonLd.newObject("Airport");
            res.reservationFor.arrivalAirport.name = arrAirport[1].trim();
            lastAirport = res.reservationFor.arrivalAirport.name;

            var arrTime = text.substr(pos + idx).match(/([0-9]{2} [A-Za-z]{3} [0-9]{4}),\s*([0-9]{2}:[0-9]{2})/);
            if (!arrTime)
                break;
            idx += arrTime.index + arrTime[0].length;
            res.reservationFor.arrivalTime = JsonLd.toDateTime(arrTime[1] + ' ' + arrTime[2], "dd MMM yyyy hh:mm", "en");

            var airline = text.substr(pos + idx).match(/([A-Z0-9]{2}) ([0-9]{3,4})\s*([A-Z][A-Za-z0-9 ]*)\n/);
            if (!airline)
                break;
            idx += airline.index + airline[0].length;
            res.reservationFor.airline = JsonLd.newObject("Airline");
            res.reservationFor.airline.iataCode = airline[1];
            res.reservationFor.airline.name = airline[3];
            res.reservationFor.flightNumber = airline[2];

            reservations.push(res);
            if (idx == 0)
                break;
            pos += idx;
        }
    }

    return reservations;
}
