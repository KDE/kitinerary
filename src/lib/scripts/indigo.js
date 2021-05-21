/*
   SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function main(pdf, node, triggerNode) {
    var res = triggerNode.result[0];
    var page = pdf.pages[triggerNode.location];

    // needs to be done manually as we don't have PDF ctime for this to work automatically
    var date = page.text.match(/Date +(\d\d \w{3} \d\d)/);
    var depTime = page.text.match(/Departure Time\s+(\d\d:\d\d)/);
    res.reservationFor.departureTime = JsonLd.toDateTime(date[1] + depTime[1], "dd MMM yyhh:mm", "en");
    var boardingTime = page.text.match(/Boarding Time\s+(\d\d:\d\d)/);
    res.reservationFor.boardingTime = JsonLd.toDateTime(date[1] + boardingTime[1], "dd MMM yyhh:mm", "en");

    var from = page.text.match(/From +(\S.+?)(?:\(|To|\s\s)/);
    res.reservationFor.departureAirport.name = from[1];
    var to = page.text.match(/To +(\S.+?)(?:\(|\s\s)/);
    res.reservationFor.arrivalAirport.name = to[1];
    return res;
}

function parseHtml(doc) {
    var pnrElem = doc.eval('//td[@class="barcode1"]')[0];
    var pnr = pnrElem.content.match(/: ([A-Z0-9]{6})/)[1];

    var reservations = new Array();
    var row = doc.eval('//table//table//tr/th/..')[2].nextSibling;
    while (!row.isNull) {
        var res = JsonLd.newFlightReservation();
        res.reservationNumber = pnr;

        var f = row.recursiveContent.match(/(.*)\n(.*)\n\s*(\d{2}:\d{2})\s*\n\s*([A-Z0-9]{2}) *(\d{1,4})\s*\n(?:.*\n)*\s*(\d{2}:\d{2})\s*\n(.*)\n\s*(\d{2}:\d{2})/);

        res.reservationFor.departureAirport.name = f[2];
        res.reservationFor.arrivalAirport.name = f[7];
        res.reservationFor.airline.iataCode = f[4];
        res.reservationFor.flightNumber = f[5];
        res.reservationFor.departureTime = JsonLd.toDateTime(f[1]+f[3], "dd MMM yyhh:mm", "en");
        res.reservationFor.arrivalTime = JsonLd.toDateTime(f[1]+f[8], "dd MMM yyhh:mm", "en");

        reservations.push(res);
        row = row.nextSibling;
    }

    return reservations;
}
