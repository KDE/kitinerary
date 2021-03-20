/*
   SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
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
    var res = Context.data[0];

    var page = pdf.pages[Context.pdfPageNumber];
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

    return res;
}
