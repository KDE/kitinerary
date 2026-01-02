/*
   SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function extractColumn(page, offset) {
    var text = page.textInRect(offset, 0, offset + 0.5, 1);
    if (!text.match(/FLIGHT \d+/))
        return null;

    var res = JsonLd.newFlightReservation();
    var images = page.imagesInRect(offset, 0, offset + 0.5, 1);
    for (var i = 0; i < images.length; ++i) {
        if (images[i].width < 300 && images[i].height < images[i].width) {
            res.reservedTicket.ticketToken = "aztecCode:" + Barcode.decodePdf417(images[i]);
            break;
        }
    }

    const dep = text.match(/Departure time.*\n+(\d{1,2} \w+ \d{4})\s+(\d{1,2}:\d{2})/);
    if (dep)
        res.reservationFor.departureTime = JsonLd.toDateTime(dep[1] + " " + dep[2], "dd MMMM yyyy hh:mm", "en");
    const arr = text.match(/Arrival time.*\n+(\d{1,2} \w+ \d{4})\s+(\d{1,2}:\d{2})/);
    if (arr)
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
