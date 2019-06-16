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

var localeMap = new Array();
localeMap['en_US'] = new Array();
localeMap['en_US']['localeMatch'] = /(?:thank you|details for booking)/;
localeMap['en_US']['airportRegExp'] = /(.*) to (.*)/;
localeMap['de_DE'] = new Array();
localeMap['de_DE']['localeMatch'] = /danke/;
localeMap['de_DE']['airportRegExp'] = /(.*) nach (.*)/;

function parseHtmlBooking(doc) {
    var reservations = new Array();

    var bookingRef = doc.eval("//title")[0].content.match(/\((\w+)\)/);
    if (!bookingRef)
        return null;

    // determine locale
    var introElem = doc.eval("//table[@class=\"email-wrapper\"]/tr[2]/td");
    var locale = "";
    for (var l in localeMap) {
        var m = introElem[0].content.match(localeMap[l]['localeMatch']);
        if (m) {
            locale = l;
            break;
        }
    }

    var elems = doc.eval("//table[@class=\"ej-flight\"]");
    for (var i = 0; i < elems.length; ++i) {
        var elem = elems[i];
        var row = elem.firstChild;
        var res = JsonLd.newObject("FlightReservation");
        res.reservationNumber = bookingRef[1];
        res.reservationFor = JsonLd.newObject("Flight");

        var airports = row.recursiveContent.match(localeMap[locale]['airportRegExp']);
        res.reservationFor.departureAirport = JsonLd.newObject("Airport");
        res.reservationFor.departureAirport.name = airports[1];

        res.reservationFor.arrivalAirport = JsonLd.newObject("Airport");
        res.reservationFor.arrivalAirport.name = airports[2];
        row = row.nextSibling;

        var flightNum = row.recursiveContent.match(/([A-Z0-9]{2,3}) ?(\d{1,4})/);
        res.reservationFor.flightNumber = flightNum[2];
        res.reservationFor.airline = JsonLd.newObject("Airline");
        res.reservationFor.airline.iataCode = flightNum[1];
        row = row.nextSibling;

        var timeCell = row.eval(".//table/tr/td");
        res.reservationFor.departureTime = JsonLd.toDateTime(timeCell[1].content.match(/\W* (.*)/)[1], "dd MMM HH:mm", locale);
        res.reservationFor.arrivalTime = JsonLd.toDateTime(timeCell[3].content.match(/\W* (.*)/)[1], "dd MMM HH:mm", locale);

        elem = elem.nextSibling;
        if (elem.attribute("class") == "ej-pax") {
            var cell = elem.firstChild.firstChild;
            res.underName = JsonLd.newObject("Person");
            res.underName.name = cell.content;

            cell = cell.nextSibling.firstChild.nextSibling;
            res.reservationFor.airplaneSeat = cell.content.match(/auto/) ? null : cell.content;
        }
        reservations.push(res);
    }

    return reservations;
}

