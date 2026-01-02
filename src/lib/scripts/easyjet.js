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

    var elems = doc.eval("//table[@class=\"ej-flight\"]");
    for (var i = 0; i < elems.length; ++i) {
        var elem = elems[i];
        var row = elem.firstChild;
        var res = JsonLd.newFlightReservation();
        res.reservationNumber = bookingRef[1];

        var airports = row.recursiveContent.match(/(.*) (?:to|nach) (.*)/);
        res.reservationFor.departureAirport.name = airports[1];

        res.reservationFor.arrivalAirport.name = airports[2];
        row = row.nextSibling;

        var flightNum = row.recursiveContent.match(/([A-Z0-9]{2,3}) ?(\d{1,4})/);
        res.reservationFor.flightNumber = flightNum[2];
        res.reservationFor.airline.iataCode = flightNum[1];
        row = row.nextSibling;

        const timeCell = row.eval(".//table/tr/td");
        res.reservationFor.departureTime = JsonLd.toDateTime(timeCell[1].content.match(/\W* (.*)/)[1], ["dd MMM HH:mm", "dd MMM yyyy HH:mm"], ["en", "de"]);
        res.reservationFor.arrivalTime = JsonLd.toDateTime(timeCell[3].content.match(/\W* (.*)/)[1], ["dd MMM HH:mm", "dd MMM yyyy HH:mm"], ["en", "de"]);

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

function parsePdfBoardingPass(pdf, node, triggerNode)
{
    var res = triggerNode.result[0];

    const page = pdf.pages[triggerNode.location];
    const rightCol = page.textInRect(0.65, 0, 1, 0.25)
        .replace(/(?:\n|  +)/g, ' ')
        .replace(/(?:from Flying|to Going|departs Flight)/g, '')
        .trim();

    const airports = rightCol.match(/\([A-Z]{3}\) (.*) \([A-Z]{3}\) (.*) (\d\d:\d\d)/);
    res.reservationFor.departureAirport.name = airports[1];
    res.reservationFor.arrivalAirport.name = airports[2];
    res.reservationFor.departureTime = JsonLd.toDateTime(airports[3], "HH:mm", "en");

    const leftCol = page.textInRect(0, 0, 0.3, 0.25);
    const boarding = leftCol.match(/(?:closes|Gate)\n+ *(\d\d:\d\d)/);
    if (boarding)
        res.reservationFor.boardingTime = JsonLd.toDateTime(boarding[1], "hh:mm", "en");

    return res;
}
