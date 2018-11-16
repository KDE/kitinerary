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

function parseHtmlBooking(doc) {
    var reservations = new Array();

    var bookingRef = doc.eval("//title")[0].content.match(/\((\w+)\)/);
    if (!bookingRef)
        return null;

    var elems = doc.eval("//table[@class=\"ej-flight\"]");
    for (var i = 0; i < elems.length; ++i) {
        var elem = elems[i];
        var row = elem.firstChild;
        var res = JsonLd.newObject("FlightReservation");
        res.reservationNumber = bookingRef[1];
        res.reservationFor = JsonLd.newObject("Flight");

        var airports = row.recursiveContent.match(/(.+) to (.+)/);
        var airportName = airports[1].match(/^(.*?)( \(Terminal (.*)\))?( \((.*) Terminal\))?$/);
        console.log(airportName);
        res.reservationFor.departureAirport = JsonLd.newObject("Airport");
        res.reservationFor.departureAirport.name = airportName[1];
        res.reservationFor.departureTerminal = airportName[3] ? airportName[3] : airportName[5];

        res.reservationFor.arrivalAirport = JsonLd.newObject("Airport");
        airportName = airports[2].match(/^(.*?)( \(Terminal (.*)\))?( \((.*) Terminal\))?$/);
        res.reservationFor.arrivalAirport.name = airportName[1];
        res.reservationFor.arrivalTerminal = airportName[3] ? airportName[3] : airportName[5];
        row = row.nextSibling;

        var flightNum = row.recursiveContent.match(/([A-Z0-9]{2,3}) ?(\d{1,4})/);
        res.reservationFor.flightNumber = flightNum[2];
        res.reservationFor.airline = JsonLd.newObject("Airline");
        res.reservationFor.airline.iataCode = flightNum[1];
        row = row.nextSibling;

        var timeCell = row.eval(".//table/tr/td");
        console.debug(timeCell[1].content, timeCell[3].content);
        res.reservationFor.departureTime = JsonLd.toDateTime(timeCell[1].content, "ddd dd MMM HH:mm", "en");
        res.reservationFor.arrivalTime = JsonLd.toDateTime(timeCell[3].content, "ddd dd MMM HH:mm", "en");

        elem = elem.nextSibling;
        if (elem.attribute("class") == "ej-pax") {
            var cell = elem.firstChild.firstChild;
            res.underName = JsonLd.newObject("Person");
            res.underName.name = cell.content;

            cell = cell.nextSibling.firstChild.nextSibling;
            res.reservationFor.airplaneSeat = cell.content.match(/auto/) ? null : cell.content;
        }
        // TODO terminal data
        reservations.push(res);
    }

    return reservations;
}

