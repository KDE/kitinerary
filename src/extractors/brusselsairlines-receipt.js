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

function extractColumn(page, offset) {
    var text = page.textInRect(offset, 0, offset + 0.5, 1);
    if (!text.match(/FLIGHT \d+/))
        return null;

    var res = JsonLd.newObject("FlightReservation");
    res.reservationFor = JsonLd.newObject("Flight");
    res.reservedTicket = JsonLd.newObject("Ticket");

    var images = page.imagesInRect(offset, 0, offset + 0.5, 1);
    for (var i = 0; i < images.length; ++i) {
        if (images[i].width < 300 && images[i].height < images[i].width) {
            res.reservedTicket.ticketToken = "aztecCode:" + Barcode.decodePdf417(images[i]);
            break;
        }
    }

    var dep = text.match(/Departure time\s(\d{1,2} \w+ \d{4})\s+(\d{1,2}:\d{2})/);
    if (dep)
        res.reservationFor.departureTime = JsonLd.toDateTime(dep[1] + " " + dep[2], "dd MMMM yyyy hh:mm", "en");
    var arr = text.match(/Arrival time\s(\d{1,2} \w+ \d{4})\s+(\d{1,2}:\d{2})/);
    if (dep)
        res.reservationFor.arrivalTime = JsonLd.toDateTime(arr[1] + " " + arr[2], "dd MMMM yyyy hh:mm", "en");

    return res;
}

function main(pdf) {
    var result = new Array();

    // each page has up to two columns, each describing one flight leg
    for (var i = 0; i < pdf.pageCount; ++i) {
        var page = pdf.pages[i];
        result.push(extractColumn(page, 0));
        result.push(extractColumn(page, 0.5));
    }

    return result;
}
