/*
   SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function main(html) {
    var reservations = new Array();

    const bookingRef = html.root.recursiveContent.match(/Booking Reference: ([A-Z0-9]{6})/)[1];

    var row = html.eval("//table//table//table//table[2]/tr")[0].nextSibling;
    var passengers = row.recursiveContent.split('\n');

    while (row && !row.isNull) {
        if (row.firstChild.recursiveContent != "Flight") {
            row = row.nextSibling;
            continue;
        }

        var res = JsonLd.newFlightReservation();
        res.reservationNumber = bookingRef;

        var flight = row.firstChild.nextSibling.recursiveContent.match(/(\S{2})(\d{4}) - \S{3} (.+)/);
        res.reservationFor.flightNumber = flight[2];
        res.reservationFor.airline.iataCode = flight[1];

        row = row.nextSibling;
        var dep = row.firstChild.nextSibling.recursiveContent.match(/([^\)]+) \((\S{3})\) (\d{2}:\d{2})/);
        res.reservationFor.departureAirport.name = dep[1];
        res.reservationFor.departureAirport.iataCode = dep[2];
        res.reservationFor.departureTime = JsonLd.toDateTime(flight[3] + dep[3], "dd MMM yyyyhh:mm", "en");

        row = row.nextSibling;
        var arr = row.firstChild.nextSibling.recursiveContent.match(/([^\)]+) \((\S{3})\) (\d{2}:\d{2})/);
        res.reservationFor.arrivalAirport.name = arr[1];
        res.reservationFor.arrivalAirport.iataCode = arr[2];
        res.reservationFor.arrivalTime = JsonLd.toDateTime(flight[3] + arr[3], "dd MMM yyyyhh:mm", "en");

        row = row.nextSibling.nextSibling;
        res.reservationFor.airline.name = row.firstChild.nextSibling.recursiveContent;

        for (i in passengers) {
            var r = JsonLd.clone(res);
            r.underName.name = passengers[i];
            reservations.push(r);
        }

        row = row.nextSibling;
    }

    return reservations;
}

function extractPdfBoardingPass(pdf, node, barcode) {
    let res = barcode.result[0];
    const text = pdf.pages[barcode.location].text;
    const times = text.match(/(\d\d:\d\d) +\d+[A-Z] +(\d\d:\d\d)/);
    res.reservationFor.departureTime = JsonLd.toDateTime(times[1], 'hh:mm', 'en');
    res.reservationFor.boardingTime = JsonLd.toDateTime(times[2], 'hh:mm', 'en');
    return res;

}
