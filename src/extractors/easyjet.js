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
        var res = JsonLd.newFlightReservation();
        res.reservationNumber = bookingRef[1];

        var airports = row.recursiveContent.match(localeMap[locale]['airportRegExp']);
        res.reservationFor.departureAirport.name = airports[1];

        res.reservationFor.arrivalAirport.name = airports[2];
        row = row.nextSibling;

        var flightNum = row.recursiveContent.match(/([A-Z0-9]{2,3}) ?(\d{1,4})/);
        res.reservationFor.flightNumber = flightNum[2];
        res.reservationFor.airline.iataCode = flightNum[1];
        row = row.nextSibling;

        var timeCell = row.eval(".//table/tr/td");
        res.reservationFor.departureTime = JsonLd.toDateTime(timeCell[1].content.match(/\W* (.*)/)[1], "dd MMM HH:mm", locale);
        res.reservationFor.arrivalTime = JsonLd.toDateTime(timeCell[3].content.match(/\W* (.*)/)[1], "dd MMM HH:mm", locale);

        elem = elem.nextSibling;
        if (elem.attribute("class") == "ej-pax") {
            var cell = elem.firstChild.firstChild;
            res.underName.name = cell.content;

            cell = cell.nextSibling.firstChild.nextSibling;
            res.reservationFor.airplaneSeat = cell.content.match(/auto/) ? null : cell.content;
        }
        reservations.push(res);
    }

    return reservations;
}

function parsePdfBoardingPass(pdf)
{
    var result = new Array();
    for (var i = 0; i < pdf.pageCount; ++i) {
        var page = pdf.pages[i];
        var images = page.images;
        for (var j = 0; j < images.length; ++j) {
            var bcbp = Barcode.decodePdf417(images[j]);
            if (!bcbp)
                continue;
            var res = JsonLd.newFlightReservation();
            res.reservedTicket.ticketToken = "aztecCode:" + bcbp;

            var rightCol = page.textInRect(0.65, 0, 1, 0.25);
            var depName = rightCol.match(/(?:from|Flying)\n\([A-Z]{3}\) ([^]*?)\n(?:to|Going)/);
            if (depName)
                res.reservationFor.departureAirport.name = depName[1];
            var arrName = rightCol.match(/(?:to|Going)\n\([A-Z]{3}\) ([^]*?)\ndeparts/);
            if (arrName)
                res.reservationFor.arrivalAirport.name = arrName[1];
            var depTime = rightCol.match(/(?:departs|Flight)\n(\d\d:\d\d)/);
            if (depTime)
                res.reservationFor.departureTime = JsonLd.toDateTime(depTime[1], "hh:mm", "en");

            var leftCol = page.textInRect(0, 0, 0.3, 0.25);
            var boarding = leftCol.match(/(?:closes|Gate)\n(\d\d:\d\d)/);
            if (boarding)
                res.reservationFor.boardingTime = JsonLd.toDateTime(boarding[1], "hh:mm", "en");
            result.push(res);
            break;
        }
    }

    return result;
}
