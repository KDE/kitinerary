/*
   Copyright (c) 2019 Volker Krause <vkrause@kde.org>

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

function main(pdf) {
    var res = Context.data[0];
    var page = pdf.pages[Context.pdfPageNumber];

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
