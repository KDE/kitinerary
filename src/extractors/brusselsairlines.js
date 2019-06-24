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

function parseAirport(airport, name)
{
    airport.name = name;
    if (name.startsWith("Brussels Airport"))
        airport.iataCode = "BRU"; // disambiguate Brussel airports
    return airport;
}

function main(html) {
    var reservations = new Array();
    var bookingRef = html.eval("//td[@class='confirmation']")[0].recursiveContent;

    var flightDetailsRoot = html.eval("//h2[text()='Flight details']")[0].nextSibling;
    if (!flightDetailsRoot)
        return null;

    while (!flightDetailsRoot.isNull && flightDetailsRoot.name != "h2") {
        if (flightDetailsRoot.name != "table") {
            flightDetailsRoot = flightDetailsRoot.nextSibling;
            continue;
        }

        var rows = flightDetailsRoot.eval("./tr");
        for (var i in rows) {
            var cell = rows[i].firstChild;
            if (cell.firstChild.isNull)
                continue;

            var res = JsonLd.newFlightReservation();
            res.reservationNumber = bookingRef;
            parseAirport(res.reservationFor.departureAirport, cell.firstChild.content);
            var depTime = cell.recursiveContent.match(/([0-9]{2} [A-Za-z]{3} [0-9]{4}),\s*([0-9]{2}:[0-9]{2})/);
            if (!depTime)
                continue;
            res.reservationFor.departureTime = JsonLd.toDateTime(depTime[1] + ' ' + depTime[2], "dd MMM yyyy hh:mm", "en");

            cell = cell.nextSibling;
            parseAirport(res.reservationFor.arrivalAirport, cell.firstChild.content);
            var arrTime = cell.recursiveContent.match(/([0-9]{2} [A-Za-z]{3} [0-9]{4}),\s*([0-9]{2}:[0-9]{2})/);
            if (!arrTime)
                continue;
            res.reservationFor.arrivalTime = JsonLd.toDateTime(arrTime[1] + ' ' + arrTime[2], "dd MMM yyyy hh:mm", "en");

            cell = cell.nextSibling;
            var airline = cell.recursiveContent.match(/([A-Z0-9]{2}) ([0-9]{3,4})\s*([A-Z][A-Za-z0-9 ]*)/);
            if (!airline)
                continue;
            res.reservationFor.airline.iataCode = airline[1];
            res.reservationFor.airline.name = airline[3];
            res.reservationFor.flightNumber = airline[2];
            reservations.push(res);
        }

        flightDetailsRoot = flightDetailsRoot.nextSibling;
    }

    return reservations;
}
